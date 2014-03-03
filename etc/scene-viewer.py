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

import bpy,subprocess,shutil,os
    
bl_info = {"name": "GamePlay 3D Scene Viewer", "category": "User"}

bpy.types.Scene.encoder_group = bpy.props.BoolProperty(name="Group Animations",default=False)
bpy.types.Scene.encoder_genmat = bpy.props.BoolProperty(name="Generate Materials",default=True)
bpy.types.Scene.encoder_path = bpy.props.StringProperty(name="Encoder Path",subtype="FILE_PATH",default="")
bpy.types.Scene.viewer_path = bpy.props.StringProperty(name="Viewer Path",subtype="FILE_PATH",default="")
bpy.types.Scene.game_path = bpy.props.StringProperty(name="Workspace",subtype="FILE_PATH",default="")

class SceneView(bpy.types.Operator):
    """the GamePlay 3D scene viewer"""
    bl_idname = "scene.gameplayview"
    bl_label = "Preview"
    
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
        sve = bpy.context.scene.viewer_path
        svp = bpy.context.scene.game_path
        if svp=='': 
            svp = os.path.dirname(sve)
        enc = bpy.context.scene.encoder_path
        sfp = os.path.join(svp,"res","scene")
        bpy.ops.export_scene.fbx(filepath=sfp+".fbx", 
                                 check_existing=True, 
                                 filter_glob="*.fbx", 
                                 use_selection=False, 
                                 global_scale=1.0, 
                                 axis_forward='Z',
                                 axis_up='Y', 
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
        subprocess.call(args)
        for img in bpy.data.images.keys():    
            if bpy.data.images[img].source=='FILE' and os.path.dirname(bpy.data.images[img].filepath)!=os.path.join(svp,"res"):
                shutil.copy(bpy.data.images[img].filepath,os.path.join(svp,"res"))
        subprocess.Popen([sve],cwd=svp)
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
        layout.operator("scene.gameplayview")

def register():
    bpy.utils.register_class(SceneView)
    bpy.utils.register_class(GameplayPanel)

def unregister():
    bpy.utils.unregister_class(SceneView)
    bpy.utils.unregister_class(GameplayPanel)
