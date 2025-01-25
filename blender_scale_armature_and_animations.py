import bpy


if bpy.context.object and bpy.context.object.type == "ARMATURE":
    armature = bpy.context.object
    scale = armature.scale[:]

    print("Armature")
    print(f"  scale = {scale}")

    print("Meshes")
    for mesh in armature.children:
        if mesh.type == "MESH":
            print(f"  {mesh.name}")
            mesh.select_set(True)

    # Apply transforms to armature and meshes
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

    # Deselect
    for obj in bpy.context.selected_objects[:]:
        obj.select_set(False)

    print("Scaling animations the same ammount as the armature scaling")
    for track in armature.animation_data.nla_tracks:
        for strip in track.strips:
            print(f"  {strip.action.name}")
            for fcurve in strip.action.fcurves:
                # I didn't find any other way of getting if the fcurve is for location
                if "location" in fcurve.data_path:
                    for keyframe in fcurve.keyframe_points:
                        currentPos = keyframe.co
                        currentPos.y *= scale[fcurve.array_index]
                        keyframe.co = currentPos

else:
    print("Armature needs to be selected")