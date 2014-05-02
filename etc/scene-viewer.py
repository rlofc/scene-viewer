# GamePlay 3D Blender Scene Viewer
# 
# Copyright 2013 Ithai Levi
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# parts of the script is copied from http://blenderartists.org/forum/showthread.php?255246-Rotate-whole-scene-around-x-90-and-apply-rotation

import bpy,subprocess,shutil,os,sys
try: import io_scene_fbx.export_fbx
except:
    print('error: io_scene_fbx.export_fbx not found.')
    # This might need to be bpy.Quit()
    raise

import math
from mathutils import Matrix
from functools import cmp_to_key

bl_info = {"name": "GamePlay 3D Scene Viewer", "category": "User"}

bpy.types.Scene.encoder_xml = bpy.props.BoolProperty(name="Generate XML",default=False)
bpy.types.Scene.rotatex = bpy.props.BoolProperty(name="Rotate X-90",default=False)
bpy.types.Scene.encoder_group = bpy.props.BoolProperty(name="Group Animations",default=False)
bpy.types.Scene.encoder_genmat = bpy.props.BoolProperty(name="Generate Materials",default=True)
bpy.types.Scene.encoder_path = bpy.props.StringProperty(name="Encoder Path",subtype="FILE_PATH",default="")
bpy.types.Scene.viewer_path = bpy.props.StringProperty(name="Viewer Path",subtype="FILE_PATH",default="")
bpy.types.Scene.game_path = bpy.props.StringProperty(name="Workspace",subtype="FILE_PATH",default="")

# SORTING HELPERS (sort list of objects, parents prior to children)
# root object -> 0, first child -> 1, ...
def myDepth(o): 
    if o == None:
        return 0
    if o.parent == None:
        return 0
    else:
        return 1 + myDepth(o.parent)

# compare: parent prior child
def myDepthCompare(a,b):
    da = myDepth(a)
    db = myDepth(b)
    if da < db:
        return -1
    elif da > db:
        return 1
    else:
        return 0

def rotateScene(ang):
    matPatch = Matrix.Rotation(ang, 4, 'X')

    # deselect everything to close edit / pose mode etc.
    bpy.context.scene.objects.active = None

    # activate all 20 layers
    for i in range(0, 20):
        bpy.data.scenes[0].layers[i] = True;

        # show all root objects
    for obj in bpy.data.objects:
        obj.hide = False;

    # make single user (otherwise import fails on instances!) --> no instance anymore
    bpy.ops.object.make_single_user(type='ALL', object=True, obdata=True)

    # prepare rotation-sensitive data
    # a) deactivate animation constraints
    # b) apply mirror modifiers
    for obj in bpy.data.objects:
        # only posed objects
        if obj.pose is not None:
            # check constraints for all bones
            for pBone in obj.pose.bones:
                for constraint in pBone.constraints:
                    # for now only deactivate limit_location
                    if constraint.type == 'LIMIT_LOCATION':
                        constraint.mute = True
        # need to activate current object to apply modifiers
        bpy.context.scene.objects.active = obj
        for modifier in obj.modifiers:
            # if you want to delete only UV_project modifiers
            if modifier.type == 'MIRROR':
                bpy.ops.object.modifier_apply(apply_as='DATA', modifier=modifier.name)

    # deselect again, deterministic behaviour!
    bpy.context.scene.objects.active = None

    # Iterate the objects in the file, only root level and rotate them
    for obj in bpy.data.objects:        
        if obj.parent != None:
            continue
        obj.matrix_world = matPatch * obj.matrix_world

    # deselect everything
    for obj in bpy.data.objects:
        obj.select = False;

    # apply all(!) transforms 
    # parent prior child
    for obj in sorted(bpy.data.objects, key=cmp_to_key(myDepthCompare)):
        obj.select = True;
        bpy.ops.object.transform_apply(rotation=True)
        # deselect again
        obj.select = False;
    #------------------------------------------------------------

class SceneView(bpy.types.Operator):
    """the GamePlay 3D scene viewer"""
    bl_idname = "scene.gameplayview"
    bl_label = "Preview"

    @staticmethod    
    def isMacApp(svp):
        return sys.platform == 'darwin' and \
               os.path.isdir(svp) and \
               (svp.endswith('.app') or svp.endswith('.app/'))

    @classmethod
    def poll(cls, context):
        svp  = bpy.context.scene.viewer_path
        enc  = bpy.context.scene.encoder_path
        gdir = bpy.context.scene.game_path
        try:
            with open(svp): pass
        except IOError:
            return False
        try:
            with open(enc): pass
        except IOError:
            return False
        if gdir!='':
            return os.path.isdir(gdir)
        else:
            return True
    
    def execute(self, context):
        
        # set mode to 'OBJECT'
        for obj in bpy.context.scene.objects:
          if obj.type == 'MESH':
            bpy.context.scene.objects.active = obj
            bpy.ops.object.mode_set(mode='OBJECT')
        
        if bpy.context.scene.rotatex:
           # Rotate -90 around the X-axis
           rotateScene(-math.pi / 2.0)
           axisForward='Y'
           axisUp='Z'
        else:
           axisForward='Z'
           axisUp='Y'
    
        sve = bpy.context.scene.viewer_path
        svp = bpy.context.scene.game_path
        resdir = ''

        macApp = SceneView.isMacApp(sve)
        if macApp and sve.endswith('.app/'):
            sve = sve[:-1]

        if svp=='':
            if macApp:
                svp = os.path.normpath(os.path.join(sve, os.pardir))
                resdir = os.path.join(sve, 'Contents', 'Resources', 'res')
            else:
                svp = os.path.dirname(sve)

        if resdir=='':
            resdir = os.path.join(svp, 'res')

        enc = bpy.context.scene.encoder_path
        barename = os.path.splitext(bpy.path.basename(bpy.context.blend_data.filepath))[0]
        sfp = os.path.join(resdir,barename)
        bpy.ops.export_scene.fbx(filepath=sfp+".fbx", 
                                 check_existing=True, 
                                 filter_glob="*.fbx", 
                                 use_selection=False, 
                                 global_scale=1.0, 
                                 axis_forward=axisForward,
                                 axis_up=axisUp,
                                 object_types={'EMPTY', 'MESH','LAMP', 'CAMERA', 'ARMATURE'}, 
                                 use_mesh_modifiers=True, 
                                 mesh_smooth_type='FACE', 
                                 use_mesh_edges=False, 
                                 use_armature_deform_only=False, 
                                 use_anim=True, 
                                 use_anim_action_all=True, 
                                 use_default_take=True, 
                                 use_anim_optimize=False, 
                                 anim_optimize_precision=6.0, 
                                 path_mode='AUTO', 
                                 batch_mode='OFF', 
                                 use_batch_own_dir=True, 
                                 use_metadata=True)
        args = [enc]
        if bpy.context.scene.encoder_group:
            args.append("-g:auto")
        else:
            args.append("-g:none")
            args.append("-g:off")
        if bpy.context.scene.encoder_genmat:
            args.append("-m")
        args.append(sfp+".fbx")
        if bpy.context.scene.encoder_xml:
                args.insert(1, "-t")
        subprocess.call(args)
        for img in bpy.data.images.keys():    
            if bpy.data.images[img].source=='FILE' and os.path.dirname(bpy.data.images[img].filepath) != resdir and os.path.exists(bpy.data.images[img].filepath):
                shutil.copy(bpy.data.images[img].filepath,resdir)
        
        if not bpy.context.scene.encoder_xml:
                # Execute scene-viewer
                if macApp:
                    subprocess.Popen(['/usr/bin/open',sve,'--args',barename],cwd=svp)
                else:
                    subprocess.Popen([sve,barename],cwd=svp)
        else:
                # Execute the platform's text editor
                pltfm = bpy.app.build_platform.decode('utf-8').lower()
                runcmd = ''
                if   pltfm.startswith('win'):
                        # runcmd = 'start "" "'+sfp+'.xml"'  # too slow on xp32
                        runcmd = 'explorer "'+sfp+'.xml"'
                        os.system('echo scene-viewer - Executing: '+runcmd)
                elif pltfm.startswith('darwin'):  # mac
                        # runcmd = '/usr/bin/open -a TextEdit "'+sfp+'.xml"'  # Use an app (TextEdit)
                        runcmd = '/usr/bin/open -t "'+sfp+'.xml"'  # Use default text editor
                        os.system('echo scene-viewer - Executing: '+runcmd)
                elif pltfm.startswith('linux'):
                        # runcmd = '( /usr/bin/gedit "'+sfp+'.xml" ) &'  # Use a custom command
                        #
                        # You may substitute the MYPREFERRED string with your graphical text editor (for example: MYPREFERRED="chromium-browser").
                        # Please don't specify a text editor with no X window interface (like Vim), or a silent process will be open in background.
                        runcmd = '''( MYPREFERRED=""; ALREADY=0 ; runcmd() { if [ $ALREADY = 0 -a ! -z "${2}" -a ! -z "`which ${1}`" ]; then
                        echo "scene-viewer - Executing:" ${1} ${2} ; "${1}" "${2}" ; ALREADY=1 ; fi } ; F="'''+sfp+'''.xml";
                        runcmd "${MYPREFERRED}" "${F}" ; runcmd "gedit" "${F}" ; runcmd "kate" "${F}" ; runcmd "leafpad" "${F}" ; ) &'''
                os.system(runcmd)  # If you want to disable the text editor, please comment this line with a '#' character.
        
        if bpy.context.scene.rotatex:
           # Rotate 90 around the X-axis
           rotateScene(math.pi / 2.0)
           
           # or use undo, so it rotates scene back (and get instances back)
           #bpy.ops.ed.undo()
           
        return {"FINISHED"}

class GameplayPanel(bpy.types.Panel):
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_label = "Gameplay"
    bl_idname = "SCENE_PT_layout"
    
    def draw(self, context):        
        layout = self.layout
        layout.prop(context.scene, "encoder_path")
        layout.prop(context.scene, "viewer_path")
        layout.prop(context.scene, "game_path")
        layout.prop(context.scene, "encoder_genmat")
        layout.prop(context.scene, "encoder_group")
        layout.prop(context.scene, "rotatex")
        layout.prop(context.scene, "encoder_xml")
        layout.operator("scene.gameplayview")

def register():
    bpy.utils.register_class(SceneView)
    bpy.utils.register_class(GameplayPanel)

def unregister():
    bpy.utils.unregister_class(SceneView)
    bpy.utils.unregister_class(GameplayPanel)
