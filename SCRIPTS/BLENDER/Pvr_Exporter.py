import bpy
import struct
import os
import math
import bmesh
import datetime
import mathutils
from mathutils import Matrix, Quaternion, Vector
from bpy.props import StringProperty, BoolProperty, EnumProperty, FloatProperty

bl_info = {
    "name": "Dreamcast PVR Format",
    "author": "Assistant",
    "version": (1, 0),
    "blender": (2, 80, 0),
    "category": "Import-Export",
}


def is_4x4(matrix):
    """Vérifie si une matrice est de taille 4x4."""
    if isinstance(matrix, Matrix):
        return len(matrix) == 4 and all(len(row) == 4 for row in matrix)
    return False

def ensure_4x4(matrix):
    """Convertit une matrice en 4x4 si nécessaire."""
    if isinstance(matrix, Matrix):
        if is_4x4(matrix):
            return matrix
        else:
            return matrix.to_4x4()
    else:
        # Si ce n'est pas une matrice Blender, créer une matrice identité 4x4
        return Matrix.Identity(4)


def debug_animation_data(armature, animations):
    """Affiche le détail des animations exportées"""
    print("\n=== Armature Debug ===")
    print(f"Armature name: {armature.name}")
    print(f"Number of bones: {len(armature.data.bones)}")
    
    # Debug des os
    print("\n=== Bones Structure ===")
    for bone in armature.data.bones:
        print(f"\nBone: {bone.name}")
        print(f"Parent: {bone.parent.name if bone.parent else 'None'}")
        print(f"Head: {bone.head_local}")
        print(f"Tail: {bone.tail_local}")
        print(f"Matrix (local space):")
        for row in bone.matrix_local:
            print(f"  {[f'{x:8.4f}' for x in row]}")

    # Debug des animations
    print("\n=== Animations Data ===")
    for anim in animations:
        print(f"\nAnimation: {anim['name']}")
        print(f"Track: {anim['track']}")
        print(f"Frames: {anim['start_frame']} - {anim['end_frame']}")
        print(f"Total frames: {len(anim['frames'])}")
        
        # Debug de quelques frames clés
        frames_to_debug = [0, len(anim['frames'])//2, len(anim['frames'])-1]
        for frame_idx in frames_to_debug:
            frame = anim['frames'][frame_idx]
            print(f"\nFrame {frame_idx} (time: {frame['time']:.3f}s):")
            
            for bone_name, bone_data in frame['bones'].items():
                print(f"  Bone {bone_name}:")
                print(f"    Position: {[f'{x:8.4f}' for x in bone_data['location']]}")
                print(f"    Rotation: {[f'{x:8.4f}' for x in bone_data['rotation']]}")
                print(f"    Scale: {[f'{x:8.4f}' for x in bone_data['scale']]}")


def find_armature_from_modifier(obj):
    """Trouve l'armature à partir des modifiers de l'objet"""
    if obj and obj.type == 'MESH':
        for modifier in obj.modifiers:
            if modifier.type == 'ARMATURE' and modifier.object:
                return modifier.object
    return None

def transform_animation_data_for_dreamcast(loc, rot, scale, parent_matrix=None):
    """
    Transforme les données d'animation pour le système de coordonnées Dreamcast.
    
    Args:
        loc: Vector position [x, y, z] (espace local)
        rot: Quaternion rotation [w, x, y, z] (espace local)
        scale: Vector scale [x, y, z] (espace local)
        parent_matrix: Matrice globale du parent (optionnelle)
    Returns:
        Tuple (new_loc, new_rot, new_scale) transformé
    """
    # Convertir la position
    x, y, z = loc
    new_loc = [x, -y, z]  # Rotation 90° sur X puis inversion Y

    # Convertir la rotation (quaternion)
    w, x, y, z = rot
    quat_local = Quaternion((w, x, y, z))

    # Si une matrice parent est fournie, convertir le quaternion en espace global
    if parent_matrix is not None:
        quat_global = parent_matrix.to_quaternion() @ quat_local
    else:
        quat_global = quat_local

    # Appliquer la transformation Dreamcast (rotation 90° sur X puis inversion Z)
    quat_rot_x = Quaternion((0.707, 0.707, 0, 0))  # Rotation de 90° sur X
    quat_inv_z = Quaternion((0, 0, 1, 0))         # Inversion de Z (rotation de 180° sur Y)
    quat_dreamcast = quat_rot_x @ quat_inv_z      # Combinaison des transformations

    new_rot = quat_dreamcast @ quat_local        # Appliquer au quaternion global
    new_rot.normalize()                           # Normaliser

    # Scale reste inchangé car il est relatif
    new_scale = list(scale)

    return (new_loc, [new_rot.w, new_rot.x, new_rot.y, new_rot.z], new_scale)

def collect_animations(armature, settings, mesh_data):
    animations = []
    
    if not armature or not settings.export_animations:
        return animations

    if not armature.animation_data or not armature.animation_data.nla_tracks:
        return animations

    current_frame = bpy.context.scene.frame_current
    
    try:
        for track in armature.animation_data.nla_tracks:
            if track.mute:
                continue

            for strip in track.strips:
                start_frame = int(strip.frame_start)
                end_frame = int(strip.frame_end)
                
                frames = []
                sample_rate = settings.animation_sample_rate
                frame_step = max(1, int(24/sample_rate))
                indexFrame = 0
                for frame in range(start_frame, end_frame + 1, frame_step):
                    bpy.context.scene.frame_set(frame)
                    #print("frame --"+str(indexFrame))
                    indexFrame = indexFrame + 1
                    frame_data = {
                        'frame': frame,
                        'time': (frame - start_frame) / 24.0,
                        'bones': {}
                    }
                    #print("--- bone ---- ")
                    for bone in armature.pose.bones:
                        if bone.name in [b['name'] for b in mesh_data['bones']]:
                            # Calculer la matrice locale
                            if bone.parent:
                                parent_matrix = bone.parent.matrix
                                local_matrix = parent_matrix.inverted() @ bone.matrix
                            else:
                                local_matrix = bone.location
                            
                            loc = bone.location
                            rot = bone.rotation_quaternion
                            scale = bone.scale
                            rotation_def = [rot.w,rot.x,rot.y,rot.z]
                            
                            #loc, rot, scale = global_matrix.decompose()
                            
                            # Transformer les coordonnées
                            new_loc, new_rot, new_scale = transform_animation_data_for_dreamcast(
                                loc, rot, scale
                            )
                            #print(f"bone value loc {loc[0],loc[1],loc[2]} rotation {rot[0],rot[1],rot[2],rot[3]}")
                            
                            frame_data['bones'][bone.name] = {
                                'location': new_loc,
                                'rotation': rotation_def,
                                'scale': new_scale,
                            }
                            #print(f"  - Rotation: {rotation_def}")
                            
                            #printf(f"framedata['bone'] {frame_data['bones'][bone.name]['location']} }")
                    
                    frames.append(frame_data)
                
                anim_data = {
                    'name': strip.name,
                    'track': track.name,
                    'start_frame': start_frame,
                    'end_frame': end_frame,
                    'frames': frames
                }
                animations.append(anim_data)

    except Exception as e:
        print(f"Error collecting animations: {str(e)}")
        import traceback
        traceback.print_exc()
    
    finally:
        bpy.context.scene.frame_set(current_frame)
    
    return animations



def transform_vertex_for_dreamcast(vertex):
    """
    Applique une rotation de 90° sur X puis une symétrie sur Z pour le système de coordonnées Dreamcast
    
    Args:
        vertex: Liste ou tuple [x, y, z]
    Returns:
        Liste [x', y', z'] transformée
    """
    x, y, z = vertex
    
    # Rotation de 90° autour de X
    # [1  0   0]   [x]   [x]
    # [0  0  -1] * [y] = [-z]
    # [0  1   0]   [z]   [y]
    x_rot = x
    y_rot = -z
    z_rot = y
    
    # Symétrie sur Z (multiplication par -1)
    return [x_rot, y_rot, -z_rot]


def transform_normal_for_dreamcast(normal):
    """
    Applique la même transformation aux normales
    """
    x, y, z = normal
    
    # Rotation de 90° autour de X
    x_rot = x
    y_rot = -z
    z_rot = y
    
    # Symétrie sur Z
    return [x_rot, y_rot, -z_rot]

def transform_uv_for_dreamcast(uv):
    """
    Applique une symétrie sur l'axe V des coordonnées UV
    
    Args:
        uv: Liste ou tuple [u, v]
    Returns:
        Liste [u, v'] transformée
    """
    u, v = uv
    # Symétrie sur V (1 - v pour inverser la coordonnée V)
    return [u, 1.0 - v]

def get_vertex_color(mesh, vert_idx):
    """Récupère la couleur d'un vertex à partir des color attributes"""
    for attr in mesh.color_attributes:
        if attr.domain == 'POINT' and attr.data_type == 'FLOAT_COLOR':
            color = attr.data[vert_idx].color
            # Debug print
            print(f"Vertex {vert_idx} color: R={color[0]:.3f}, G={color[1]:.3f}, B={color[2]:.3f}, A={color[3]:.3f}")
            return color
    return None


def matrix_to_list(matrix):
    """Convertit une matrice Blender en liste de 16 flottants"""
    # Aplatit la matrice 4x4 en liste de 16 éléments
    return [v for row in matrix for v in row]

def bone_matrix_to_list(bone_matrix):
    """Convertit une matrice d'os en liste de 16 flottants"""
    if len(bone_matrix) == 16:
        return bone_matrix  # Déjà au bon format
    elif hasattr(bone_matrix, 'to_4x4'):
        return matrix_to_list(bone_matrix.to_4x4())
    else:
        # Crée une matrice identité si la conversion échoue
        return [1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1]

def write_bone_data(f, bone_data, index):
    """Export un bone au format binaire"""
    # Nom (4 bytes length + 32 bytes fixed)
    name = bone_data['name'].encode('utf-8')
    name_length = len(name)
    f.write(struct.pack('I', name_length))
    name_buffer = bytearray(32)  # Buffer de 32 bytes initialisé à 0
    name_buffer[:name_length] = name  # Copie le nom au début
    f.write(name_buffer)

    # Matrices (3 x 16 floats = 192 bytes)
    def write_matrix(matrix):
        if isinstance(matrix, mathutils.Matrix):
            data = [x for row in matrix for x in row]
        else:
            data = list(matrix) if matrix else [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]
        f.write(struct.pack('16f', *data))

    write_matrix(bone_data.get('matrix'))
    write_matrix(bone_data.get('bind_matrix'))
    write_matrix(bone_data.get('inv_bind_matrix'))

    # Parent index (4 bytes)
    parent_idx = bone_data.get('parent_index', -1)
    f.write(struct.pack('i', parent_idx))


def export_bones(context, filename, armature):
    bones = []
    for bone in armature.data.bones:
        bone_data = {
            'name': bone.name,
            'matrix': bone.matrix_local,
            'bind_matrix': bone.matrix,
            'inv_bind_matrix': bone.matrix_local.inverted(),
            'parent_index': list(armature.data.bones).index(bone.parent) if bone.parent else -1
        }
        bones.append(bone_data)

    with open(filename + "_debug_bones.txt", 'w') as f:
        for i, bone in enumerate(bones):
            f.write(f"Bone {i}: {bone['name']}\n")
            f.write(f"Parent: {bone['parent_index']}\n")
            f.write("Matrix:\n")
            f.write(str(bone['matrix']))
            f.write("\n\n")

    return bones

def collect_vertex_weights(vertex, obj, bone_mapping):
    """
    Collecte les poids et indices des os pour un vertex
    """
    weights = []
    bone_indices = []  # Liste pour stocker les indices de bones associés au vertex
    
    for group in vertex.groups:
        # Récupère le nom du groupe de vertex (qui correspond au nom de l'os)
        group_name = obj.vertex_groups[group.group].name
        if group_name in bone_mapping:
            bone_index = bone_mapping[group_name]
            weight = group.weight
            
            # Ajoute l'indice de bone et le poids à la liste
            weights.append(weight)
            bone_indices.append(bone_index)
    
    # Trie les poids par ordre décroissant (optionnel)
    # Si vous souhaitez conserver l'ordre des poids les plus importants
    if weights and bone_indices:
        sorted_indices = sorted(range(len(weights)), key=lambda i: weights[i], reverse=True)
        weights = [weights[i] for i in sorted_indices]
        bone_indices = [bone_indices[i] for i in sorted_indices]
    
    # Limite à 4 poids maximum (ou le nombre d'influences max défini dans les paramètres)
    max_influences = 4  # Vous pouvez remplacer par une variable si nécessaire
    weights = weights[:max_influences]
    bone_indices = bone_indices[:max_influences]
    
    # Complète avec des poids nuls si nécessaire
    while len(weights) < max_influences:
        weights.append(0.0)
        bone_indices.append(0)  # Indice 0 pour un bone par défaut (ou -1 si vous préférez)
    
    return weights, bone_indices


def matrix_to_list(matrix):
    """
    Convertit une matrice Blender en liste de 16 flottants (4x4)
    """
    if isinstance(matrix, mathutils.Matrix):
        # Si c'est déjà une matrice 4x4
        if len(matrix.col) == 4 and len(matrix.row) == 4:
            return [x for row in matrix for x in row]
        # Sinon, la convertir en 4x4
        mat4 = matrix.to_4x4()
        return [x for row in mat4 for x in row]
    elif isinstance(matrix, list):
        # Si c'est déjà une liste de 16 éléments
        if len(matrix) == 16:
            return matrix
        # Si c'est une liste de listes (matrice)
        elif len(matrix) == 4 and all(len(row) == 4 for row in matrix):
            return [x for row in matrix for x in row]
    
    # En cas d'erreur, retourner une matrice identité
    return [1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1]

def write_pvr_dreamcast_binary(context, filepath, settings, mesh_data, obj):
    print("\n=== Writing Binary PVR File ===")
    
    with open(filepath, 'wb') as f:
        # Header (PVRD + version)
        f.write(b'PVRD')  # Signature magic
        f.write(struct.pack('<I', 1))  # Version 1
        
        # Nom du modèle (32 bytes fixe)
        model_name = obj.name.encode('utf-8')[:31]
        name_buffer = bytearray(32)
        name_buffer[:len(model_name)] = model_name
        f.write(name_buffer)
        
        # Compteurs globaux
        submesh_count = len(mesh_data['submeshes'])
        bone_count = len(mesh_data.get('bones', []))
        animation_count = len(mesh_data.get('animations', []))
        f.write(struct.pack('<3I', submesh_count, bone_count, animation_count))

        # Pour chaque submesh, écriture des triangles
        for submesh_idx, submesh in enumerate(mesh_data['submeshes']):
            print(f"\nWriting submesh {submesh_idx}:")
            print(f"- Vertices: {len(submesh['vertices'])}")
            print(f"- Indices: {len(submesh['indices'])}")
            print(f"- Triangles: {len(submesh['indices']) // 3}")

            # Nom de la texture (32 bytes fixe)
            texture_name = submesh.get('texture_name', '').encode('utf-8')[:31]
            name_buffer = bytearray(32)
            name_buffer[:len(texture_name)] = texture_name
            f.write(name_buffer)
            
            # Compteurs du submesh
            triangle_count = len(submesh['indices']) // 3
            f.write(struct.pack('<I', triangle_count))
            
            # Pour chaque triangle
            for tri_idx in range(0, len(submesh['indices']), 3):
                # Pour chaque vertex du triangle
                for v_idx in range(3):
                    vertex_idx = submesh['indices'][tri_idx + v_idx]
                    
                    # Vérifiez que vertex_idx est valide
                    if vertex_idx >= len(submesh['vertices']):
                        print(f"Erreur : vertex_idx {vertex_idx} est hors limites (max = {len(submesh['vertices']) - 1})")
                        continue

                    # flags (0xE0000000 pour début/milieu, 0xF0000000 pour fin)
                    flags = 0xF0000000 if v_idx == 2 else 0xE0000000
                    f.write(struct.pack('<I', flags))
                    
                    # Position (x, y, z)
                    pos = submesh['vertices'][vertex_idx]
                    f.write(struct.pack('<3f', *pos))
                    
                    # UV (u, v)
                    if submesh['uvs'] and vertex_idx < len(submesh['uvs']):
                        uv = submesh['uvs'][vertex_idx]
                        f.write(struct.pack('<2f', *uv))
                    else:
                        f.write(struct.pack('<2f', 0.0, 0.0))
                    
                    # Couleur ARGB
                    color = submesh['colors'][vertex_idx] if submesh['colors'] and vertex_idx < len(submesh['colors']) else 0xFFFFFFFF
                    f.write(struct.pack('<I', color))
                    
                    # Offset color (oargb - non utilisé)
                    f.write(struct.pack('<I', 0))
                    
                    # Normal pour ce vertex
                    if submesh['normals'] and vertex_idx < len(submesh['normals']):
                        normal = submesh['normals'][vertex_idx]
                        f.write(struct.pack('<3f', *normal))
                    else:
                        f.write(struct.pack('<3f', 0.0, 0.0, 1.0))
                    
                    # Poids des vertex
                    if 'vertex_weights' in submesh and 'vertex_bone_indices' in submesh:
                        if vertex_idx < len(submesh['vertex_weights']) and vertex_idx < len(submesh['vertex_bone_indices']):
                            weights = submesh['vertex_weights'][vertex_idx]
                            indices = submesh['vertex_bone_indices'][vertex_idx]
                            
                            # Compter le nombre réel de poids non nuls
                            num_real_weights = 0
                            for weight in weights:
                                if weight > 0.0001:  # Seuil pour éviter les valeurs trop proches de 0
                                    num_real_weights += 1
                            
                            # Écrire le nombre réel de poids
                            f.write(struct.pack('<I', num_real_weights))
                            
                            # Écrire les 4 emplacements pour garder la structure fixe
                            for weight_idx in range(4):
                                bone_idx = indices[weight_idx] if weight_idx < len(indices) else 0
                                weight = weights[weight_idx] if weight_idx < len(weights) else 0.0
                                f.write(struct.pack('<If', bone_idx, weight))
                        else:
                            # Pas de poids - écrire 0 poids mais quand même la structure pour 4 emplacements
                            f.write(struct.pack('<I', 0))  # 0 poids
                            for _ in range(4):  # 4 emplacements vides
                                f.write(struct.pack('<If', 0, 0.0))
                    else:
                        # Pas de poids - écrire 0 poids mais quand même la structure pour 4 emplacements
                        f.write(struct.pack('<I', 0))  # 0 poids
                        for _ in range(4):  # 4 emplacements vides
                            f.write(struct.pack('<If', 0, 0.0))
        
        # Écriture des os
        if bone_count > 0:
            for bone in mesh_data['bones']:
                # Nom de l'os (32 bytes fixe)
                bone_name = bone['name'].encode('utf-8')[:31]
                name_buffer = bytearray(32)
                name_buffer[:len(bone_name)] = bone_name
                f.write(name_buffer)
                
                # Index du parent (-1 si pas de parent)
                f.write(struct.pack('<i', bone['parent_index']))
                
                # Matrices de transformation
                f.write(struct.pack('<16f', *matrix_to_list(bone['matrix'])))
                f.write(struct.pack('<16f', *matrix_to_list(bone['bind_matrix'])))
                f.write(struct.pack('<16f', *matrix_to_list(bone['inv_bind_matrix'])))
        
        # Écriture des animations
        if animation_count > 0:
            for anim in mesh_data['animations']:
                # Nom de l'animation (32 bytes fixe)
                anim_name = anim['name'].encode('utf-8')[:31]
                name_buffer = bytearray(32)
                name_buffer[:len(anim_name)] = anim_name
                f.write(name_buffer)
                
                # Frames de début et fin
                f.write(struct.pack('<2I', anim['start_frame'], anim['end_frame']))
                
                # Nombre de frames
                frame_count = len(anim['frames'])
                f.write(struct.pack('<I', frame_count))
                
                # Pour chaque frame
                for frame in anim['frames']:
                    f.write(struct.pack('<f', frame['time']))
                    bone_count = len(frame['bones'])
                    f.write(struct.pack('<I', bone_count))
                    
                    for bone_name, bone_data in frame['bones'].items():
                        bone_name_enc = bone_name.encode('utf-8')[:31]
                        name_buffer = bytearray(32)
                        name_buffer[:len(bone_name_enc)] = bone_name_enc
                        f.write(name_buffer)
                        
                        f.write(struct.pack('<3f', *bone_data['location']))
                        f.write(struct.pack('<4f', *bone_data['rotation']))
                        f.write(struct.pack('<3f', *bone_data['scale']))

    print(f"Binary export completed: {filepath}")
    return {'FINISHED'}
    
def write_pvr_dreamcast_text(context, filepath, settings, mesh_data, obj):
    print("\n=== Writing Text PVR File ===")
    
    with open(filepath, 'w') as f:
        # En-tête
        f.write("# PVR Dreamcast Model Export\n")
        f.write(f"# Generated: {datetime.datetime.now()}\n")
        f.write(f"# Model: {obj.name}\n\n")
        
        # Header
        f.write("# === Header ===\n")
        f.write(f"Magic: PVRD\n")
        f.write(f"Version: 1\n")
        f.write(f"Model Name: {obj.name}\n")
        f.write(f"Mesh Count: {len(mesh_data['submeshes'])}\n")
        f.write(f"Bone Count: {len(mesh_data.get('bones', []))}\n")
        f.write(f"Animation Count: {len(mesh_data.get('animations', []))}\n\n")
        
        # Pour chaque submesh
        for submesh_idx, submesh in enumerate(mesh_data['submeshes']):
            f.write(f"\n# === Submesh {submesh_idx} ===\n")
            f.write(f"Texture Name: {submesh.get('texture_name', 'None')}\n")
            f.write(f"Triangle Count: {len(submesh['indices']) // 3}\n")
            
            # Écriture des triangles et leurs vertices
            vertex_count = len(submesh['indices'])
            f.write(f"\n# Vertices ({vertex_count} total)\n")
            f.write("# Format: flags x y z u v argb oargb\n")
            f.write("# followed by: normal(x y z) weights(count [bone_idx weight]*4)\n")
            
            for tri_idx in range(0, len(submesh['indices']), 3):
                f.write(f"\n# Triangle {tri_idx // 3}\n")
                
                # Pour chaque vertex du triangle
                for v_idx in range(3):
                    vertex_idx = submesh['indices'][tri_idx + v_idx]
                    
                    # Vérifiez que vertex_idx est valide
                    if vertex_idx >= len(submesh['vertices']):
                        print(f"Erreur : vertex_idx {vertex_idx} est hors limites (max = {len(submesh['vertices']) - 1})")
                        continue

                    # flags (0xE0000000 pour début/milieu, 0xF0000000 pour fin)
                    flags = 0xF0000000 if v_idx == 2 else 0xE0000000
                    
                    # Position (x, y, z)
                    pos = submesh['vertices'][vertex_idx]
                    
                    # UV (u, v)
                    uv = submesh['uvs'][vertex_idx] if submesh['uvs'] and vertex_idx < len(submesh['uvs']) else (0.0, 0.0)
                    
                    # Couleur ARGB
                    color = submesh['colors'][vertex_idx] if submesh['colors'] and vertex_idx < len(submesh['colors']) else 0xFFFFFFFF
                    
                    # Écriture du vertex
                    f.write(f"v{v_idx}: flags=0x{flags:08X}")
                    f.write(f" pos=({pos[0]:.6f}, {pos[1]:.6f}, {pos[2]:.6f})")
                    f.write(f" uv=({uv[0]:.6f}, {uv[1]:.6f})")
                    f.write(f" argb=0x{color:08X}")
                    f.write(" oargb=0x00000000")
                    
                    # Normal pour ce vertex
                    if submesh['normals'] and vertex_idx < len(submesh['normals']):
                        normal = submesh['normals'][vertex_idx]
                        f.write(f"\n    normal=({normal[0]:.6f}, {normal[1]:.6f}, {normal[2]:.6f})")
                    else:
                        f.write(f"\n    normal=(0.000000, 0.000000, 1.000000)")
                    
                    # Poids des vertex
                    if 'vertex_weights' in submesh and 'vertex_bone_indices' in submesh:
                        if vertex_idx < len(submesh['vertex_weights']) and vertex_idx < len(submesh['vertex_bone_indices']):
                            weights = submesh['vertex_weights'][vertex_idx]
                            indices = submesh['vertex_bone_indices'][vertex_idx]
                            
                            # Compter le nombre réel de poids non nuls
                            num_weights = sum(1 for w in weights if w > 0.0001)
                            f.write(f"\n    weights={num_weights}")
                            
                            # Écriture des couples (indice, poids)
                            for idx, (bone_idx, weight) in enumerate(zip(indices, weights)):
                                f.write(f" [{bone_idx}:{weight:.4f}]")
                        else:
                            f.write(f"\n    weights=0")
                    else:
                        f.write(f"\n    weights=0")
                    
                    f.write('\n')
                f.write('\n')
        
        # Écriture des os
        if mesh_data.get('bones'):
            f.write("\n# === Bones ===\n")
            f.write("# Format: name parent_index matrix[16] bind_matrix[16] inv_bind_matrix[16]\n")
            
            for bone_idx, bone in enumerate(mesh_data['bones']):
                f.write(f"\nBone {bone_idx}: {bone['name']}\n")
                f.write(f"Parent Index: {bone['parent_index']}\n")
                
                # Matrices
                f.write("Local Matrix:\n")
                matrix = matrix_to_list(bone['matrix'])
                for row in range(4):
                    f.write("  ")
                    f.write(" ".join(f"{x:8.4f}" for x in matrix[row*4:(row+1)*4]))
                    f.write("\n")
                
                f.write("Bind Matrix:\n")
                bind_matrix = matrix_to_list(bone['bind_matrix'])
                for row in range(4):
                    f.write("  ")
                    f.write(" ".join(f"{x:8.4f}" for x in bind_matrix[row*4:(row+1)*4]))
                    f.write("\n")
                
                f.write("Inverse Bind Matrix:\n")
                inv_matrix = matrix_to_list(bone['inv_bind_matrix'])
                for row in range(4):
                    f.write("  ")
                    f.write(" ".join(f"{x:8.4f}" for x in inv_matrix[row*4:(row+1)*4]))
                    f.write("\n")
        
        # Écriture des animations
        if mesh_data.get('animations'):
            f.write("\n# === Animations ===\n")
            
            for anim_idx, anim in enumerate(mesh_data['animations']):
                f.write(f"\nAnimation {anim_idx}: {anim['name']}\n")
                f.write(f"Frame Range: {anim['start_frame']} - {anim['end_frame']}\n")
                f.write(f"Frame Count: {len(anim['frames'])}\n\n")
                
                for frame_idx, frame in enumerate(anim['frames']):
                    f.write(f"Frame {frame_idx} (Time: {frame['time']:.3f}s)\n")
                    f.write(f"Bone Count: {len(frame['bones'])}\n")
                    
                    for bone_name, bone_data in frame['bones'].items():
                        f.write(f"  Bone: {bone_name}\n")
                        f.write(f"    Position: ({bone_data['location'][0]:.6f}, {bone_data['location'][1]:.6f}, {bone_data['location'][2]:.6f})\n")
                        f.write(f"    Rotation: ({bone_data['rotation'][0]:.6f}, {bone_data['rotation'][1]:.6f}, {bone_data['rotation'][2]:.6f}, {bone_data['rotation'][3]:.6f})\n")
                        f.write(f"    Scale: ({bone_data['scale'][0]:.6f}, {bone_data['scale'][1]:.6f}, {bone_data['scale'][2]:.6f})\n")

    print(f"Text export completed: {filepath}")
    return {'FINISHED'}
        
    

def collect_bones(armature):
    if not armature:
        return []
    
    bones = []
    
    # Matrice de transformation pour la Dreamcast
    rotation_matrix = Matrix.Rotation(math.radians(90), 4, 'X')  # Rotation de 90° sur X
    invert_y_matrix = Matrix.Scale(-1, 4, (0, 1, 0))  # Symétrie sur Y
    invert_z_matrix = Matrix.Scale(-1, 4, (0, 0, 1))
    print(f"invert_y_matrix {invert_y_matrix}")
    print(f"invert_y_matrix {invert_z_matrix}")
    dreamcast_transform_RootBone = rotation_matrix @ invert_z_matrix  # Combinaison des transformations
    dreamcast_transform_ChildrenBone = rotation_matrix @ invert_y_matrix

    # Parcourir tous les bones
    for bone in armature.data.bones:
        # Convertir les matrices en 4x4
        matrix_local = ensure_4x4(bone.matrix_local)
        
        # Appliquer la transformation Dreamcast
        if not bone.parent:
            # Root bone : appliquer dreamcast_transform_RootBone
            matrix_local_dreamcast = dreamcast_transform_RootBone @ matrix_local
        else:
            # Bone enfant : appliquer dreamcast_transform_ChildrenBone
            matrix_local_dreamcast = dreamcast_transform_ChildrenBone @ matrix_local

        # Calculer la matrice bind et son inverse
        bind_matrix_dreamcast = matrix_local_dreamcast.copy()
        inv_bind_matrix_dreamcast = ensure_4x4(matrix_local_dreamcast.inverted())

        # Afficher les matrices pour le débogage
        """
        print(f"-- Bone {bone.name} --")
        print("-- matrix_local_dreamcast --")
        print(matrix_local_dreamcast)
        print("-- bind_matrix_dreamcast --")
        print(bind_matrix_dreamcast)
        print("-- inv_bind_matrix_dreamcast --")
        print(inv_bind_matrix_dreamcast)
        """

        # Stocker les données du bone
        bone_data = {
            'name': bone.name,
            'matrix': matrix_local_dreamcast,
            'bind_matrix': bind_matrix_dreamcast,
            'inv_bind_matrix': inv_bind_matrix_dreamcast,
            'parent_index': list(armature.data.bones).index(bone.parent) if bone.parent else -1,
            'head': bone.head_local,  # Position locale
            'tail': bone.tail_local,  # Position locale
            'length': bone.length  # Longueur du bone
        }
        bones.append(bone_data)
    
    return bones

def write_pvr_dreamcast(context, filepath, settings):
    print("\n-------------------------------------")
    print("-------------------------------------")
    print("-------- EXPORT PVR DREAMCAST -------\n")
    print("\n -- FN WRITE PVR DREAMCAST\n")

    obj = context.active_object
    if not obj or obj.type != 'MESH':
        raise Exception("Sélectionnez un objet mesh")

    mesh = obj.data
    armature = obj.find_armature()

    # Récupérer les données globales
    mesh_data = {
        'submeshes': [],
        'bones': collect_bones(armature) if armature else [],
        'animations': []
    }

    # Créer le mapping des os avant la boucle des matériaux
    bone_mapping = {}
    if mesh_data['bones']:
        bone_mapping = {bone['name']: i for i, bone in enumerate(mesh_data['bones'])}

    # Pour chaque matériau
    for mat_idx, mat_slot in enumerate(obj.material_slots):
        material = mat_slot.material
        if not material:
            continue

        print(f"\nProcessing material: {material.name}")

        # Initialiser le submesh
        submesh = {
            'vertices': [],
            'normals': [],
            'uvs': [],
            'colors': [],
            'indices': [],  # Indices des triangles (locaux au submesh)
            'vertex_weights': [],
            'vertex_bone_indices': [],
            'material_name': material.name,
            'texture_name': ''
        }

        # Créer un dictionnaire pour mapper les indices globaux aux indices locaux
        global_to_local_index = {}
        local_index = 0

        # Pour chaque face du matériau
        for poly in mesh.polygons:
            if poly.material_index != mat_idx:
                continue

            # Pour chaque vertex du polygone
            for loop_idx in poly.loop_indices:
                vert_idx = mesh.loops[loop_idx].vertex_index

                # Si le vertex n'a pas encore été ajouté au submesh
                if vert_idx not in global_to_local_index:
                    global_to_local_index[vert_idx] = local_index
                    local_index += 1

                    # Ajouter les données du vertex au submesh
                    vertex = mesh.vertices[vert_idx]
                    submesh['vertices'].append(transform_vertex_for_dreamcast(list(vertex.co)))
                    submesh['normals'].append(transform_normal_for_dreamcast(list(vertex.normal)))

                    if mesh.uv_layers.active and loop_idx < len(mesh.uv_layers.active.data):
                        uv = list(mesh.uv_layers.active.data[loop_idx].uv)
                        submesh['uvs'].append(transform_uv_for_dreamcast(uv))
                    else:
                        submesh['uvs'].append((0.0, 0.0))  # Valeur par défaut si les UVs sont manquants

                    # Gestion des vertex colors
                    vertex_color = get_vertex_color(mesh, vert_idx)
                    if vertex_color:
                        r = int(vertex_color[0] * 255)
                        g = int(vertex_color[1] * 255)
                        b = int(vertex_color[2] * 255)
                        a = int(vertex_color[3] * 255)

                        argb = (a << 24) | (r << 16) | (g << 8) | b
                        submesh['colors'].append(argb)
                    else:
                        submesh['colors'].append(0xFFFFFFFF)

                    # Gestion des poids des vertex
                    if armature:
                        weights = []
                        weights, bone_indices = collect_vertex_weights(vertex, obj, bone_mapping)

                        # Ajoute les poids et indices de bones au submesh
                        submesh['vertex_weights'].append(weights)
                        submesh['vertex_bone_indices'].append(bone_indices)

                # Ajouter l'indice local du vertex au submesh
                submesh['indices'].append(global_to_local_index[vert_idx])

        # Ajouter le submesh s'il contient des données
        if submesh['vertices'] and submesh['indices']:
            print(f"\nFinished submesh for material {material.name}:")
            print(f"- Vertices: {len(submesh['vertices'])}")
            print(f"- Indices: {len(submesh['indices'])}")
            print(f"- Triangles: {len(submesh['indices']) // 3}")
            mesh_data['submeshes'].append(submesh)
        else:
            print(f"Submesh for material {material.name} is empty and will not be exported.")

    # Collecter les animations si on a une armature
    if armature and mesh_data['bones']:
        mesh_data['animations'] = collect_animations(armature, settings, mesh_data)

    # Export du fichier selon le format choisi
    if settings.export_format == 'BINARY':
        return write_pvr_dreamcast_binary(context, filepath, settings, mesh_data, obj)
    else:  # TEXT
        return write_pvr_dreamcast_text(context, filepath + ".txt", settings, mesh_data, obj)
    
    

class PVRDreamcastExportSettings(bpy.types.PropertyGroup):
    export_path: StringProperty(
        name="Dossier d'export",
        description="Chemin absolu vers le dossier d'export",
        default="",
        maxlen=1024,
        subtype='DIR_PATH',
        update=lambda self, context: self.validate_path(context)
    )
    
    def validate_path(self, context):
        """Valide et convertit le chemin si nécessaire"""
        if self.export_path:
            try:
                # Convertit le chemin relatif en absolu
                abs_path = os.path.abspath(bpy.path.abspath(self.export_path))
                # Normalise le chemin
                self.export_path = os.path.normpath(abs_path)
            except:
                pass
    
    filename: StringProperty(
        name="Nom du fichier",
        description="Nom du fichier d'export (sans extension)",
        default="model",
        maxlen=255
    )
    
    # Options de Mesh
    apply_transform: BoolProperty(
        name="Appliquer les transformations",
        description="Applique les transformations de l'objet au mesh",
        default=True
    )
    
    triangulate: BoolProperty(
        name="Trianguler",
        description="Convertit toutes les faces en triangles",
        default=True
    )
    
    generate_strips: BoolProperty(
        name="Générer des Triangle Strips",
        description="Optimise la géométrie en strips (recommandé pour Dreamcast)",
        default=True
    )
    
    # Options d'export
    export_uvs: BoolProperty(
        name="Exporter les UVs",
        description="Exporte les coordonnées de texture",
        default=True
    )
    
    export_colors: BoolProperty(
        name="Exporter les vertex colors",
        description="Exporte les couleurs des vertices",
        default=True
    )
    
    export_bones: BoolProperty(
        name="Exporter les os",
        description="Exporte l'armature et les poids",
        default=True
    )
    
    export_all_bones: BoolProperty(
        name="Exporter tous les os",
        description="Exporte aussi les os non-deform",
        default=False
    )
    
    export_animations: BoolProperty(
        name="Exporter les animations",
        description="Exporte les animations de l'armature",
        default=True
    )
    
    # Paramètres avancés
    max_influences: bpy.props.IntProperty(
        name="Influences max par vertex",
        description="Nombre maximum d'os influençant un vertex",
        default=4,
        min=1,
        max=4
    )
    
    weight_threshold: bpy.props.FloatProperty(
        name="Seuil des poids",
        description="Ignore les poids en dessous de cette valeur",
        default=0.01,
        min=0.0,
        max=1.0
    )
    
    animation_sample_rate: bpy.props.FloatProperty(
        name="Framerate d'animation",
        description="Nombre d'images par seconde pour l'animation",
        default=24.0,
        min=1.0,
        max=60.0
    )
    
    export_muted_tracks: bpy.props.BoolProperty(
        name="Exporter pistes muettes",
        description="Exporter aussi les pistes d'animation muettes",
        default=False
    )
    
    export_hidden_tracks: bpy.props.BoolProperty(
        name="Exporter pistes cachées",
        description="Exporter aussi les pistes d'animation cachées",
        default=False
    )
    
    export_format: bpy.props.EnumProperty(
        name="Format d'export",
        description="Choisir le format d'export",
        items=[
            ('BINARY', "Binaire", "Format binaire optimisé pour la Dreamcast"),
            ('TEXT', "Texte", "Format texte pour debug et vérification"),
        ],
        default='BINARY'
    )

class VIEW3D_PT_dreamcast_export(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Dreamcast Export'
    bl_label = "Dreamcast Model Exporter"
    bl_options = {'DEFAULT_CLOSED'}
    
    def draw(self, context):
        layout = self.layout
        settings = context.scene.pvr_export_settings
        obj = context.active_object
        # Bouton d'export
        row = layout.row()
        row.scale_y = 2.0
        row.operator(ExportPVRDreamcast.bl_idname, text="EXPORTER", icon='EXPORT')
        
        # Paramètres d'export principaux
        box = layout.box()
        box.label(text="Paramètres d'export", icon='SETTINGS')
        box.prop(settings, "export_path")
        box.prop(settings, "filename")
        
        # Format d'export
        box = layout.box()
        box.label(text="Format", icon='FILE')
        box.prop(settings, "export_format")
        
        # Options de Mesh
        box = layout.box()
        box.label(text="Options de Mesh", icon='MESH_DATA')
        box.prop(settings, "apply_transform")
        box.prop(settings, "triangulate")
        box.prop(settings, "generate_strips")
        
        # Options d'export
        box = layout.box()
        box.label(text="Options d'Export", icon='EXPORT')
        box.prop(settings, "export_uvs")
        box.prop(settings, "export_colors")
        box.prop(settings, "export_bones")
        
        if settings.export_bones:
            sub_box = box.box()
            sub_box.prop(settings, "export_all_bones")
            sub_box.prop(settings, "max_influences")
            sub_box.prop(settings, "weight_threshold")
            sub_box.prop(settings, "export_animations")
            
            if settings.export_animations:
                sub_box.prop(settings, "animation_sample_rate")
        
        # Informations sur l'objet
        if obj and obj.type == 'MESH':
            box = layout.box()
            box.label(text="Informations", icon='INFO')
            box.label(text=f"Vertices: {len(obj.data.vertices)}")
            box.label(text=f"Faces: {len(obj.data.polygons)}")
            if obj.data.uv_layers:
                box.label(text=f"UV Layers: {len(obj.data.uv_layers)}")
            if obj.data.vertex_colors:
                box.label(text=f"Vertex Colors: {len(obj.data.vertex_colors)}")
        
        
        
        # Status
        if hasattr(context.scene, "pvr_export_status"):
            box = layout.box()
            box.label(text="Statut:", icon='INFO')
            box.label(text=context.scene.pvr_export_status)
            
            
class ExportPVRDreamcast(bpy.types.Operator):
    """Export en format PVR Dreamcast"""
    bl_idname = "export.pvr_dreamcast"
    bl_label = "Export PVR Dreamcast (.pvr)"
    
    def execute(self, context):
        try:
            settings = context.scene.pvr_export_settings
            
            # Vérification des chemins
            if not settings.export_path:
                self.report({'ERROR'}, "Veuillez définir un dossier d'export")
                return {'CANCELLED'}
            
            if not settings.filename:
                self.report({'ERROR'}, "Veuillez définir un nom de fichier")
                return {'CANCELLED'}
            
            # Construction du chemin complet
            filepath = os.path.join(settings.export_path, settings.filename + ".pvr")
            
            # Convertir le chemin relatif en absolu si nécessaire
            if filepath.startswith('//'):
                blend_path = os.path.dirname(bpy.data.filepath)
                rel_path = filepath[2:]  # Supprime '//'
                abs_path = os.path.abspath(os.path.join(blend_path, rel_path))
            else:
                abs_path = os.path.abspath(filepath)
            
            # Crée les dossiers si ils n'existent pas
            os.makedirs(os.path.dirname(abs_path), exist_ok=True)
            
            # Export avec le chemin absolu
            
            result = write_pvr_dreamcast(context, abs_path, settings)
            
            if result == {'FINISHED'}:
                context.scene.pvr_export_status = f"Export réussi : {abs_path}"
                self.report({'INFO'}, f"Export réussi vers {abs_path}")
            return result
            
        except Exception as e:
            context.scene.pvr_export_status = f"Erreur : {str(e)}"
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}

# Registration
classes = (
    PVRDreamcastExportSettings,
    VIEW3D_PT_dreamcast_export,
    ExportPVRDreamcast,
)

def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.Scene.pvr_export_settings = bpy.props.PointerProperty(type=PVRDreamcastExportSettings)
    bpy.types.Scene.pvr_export_status = bpy.props.StringProperty(default="")

def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    del bpy.types.Scene.pvr_export_settings
    del bpy.types.Scene.pvr_export_status

if __name__ == "__main__":
    register()