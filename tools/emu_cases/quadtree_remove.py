"""QuadTree::Remove 0x004dcf20, taking an object out of the cells of its code (tools/emu_diff.py case).

The trees come from tools/emu_cases/quadtree_insert.py. Most cases first make each selected
cell's path exist (all of it, or broken off at a missing child or an ungrown node) and put the
object into the leaf's item list: alone (the leaf empties and is deleted unless it still has
children), next to other objects, in the second block, or twice in one cell. Others remove an
object that is not in the tree, or use code 0xf.

QuadTreeNode::RemoveObject (0x004dd1b0) and the node's deleting destructor (vtable 0x00557648
slot 0, 0x004dd0a0 -> 0x004dd0c0) run as retail code in both bodies; BlockAllocator::Free
(0x00424050) and operator delete (0x004a30c0) are logged stubs.
"""
import copy

from tools.emu_cases.quadtree_insert import (
    MAX_DEPTH, cell_origins, make_code, path_indices, random_node, write_objects, write_quadtree)

RETAIL_VA = 0x004dcf20
SYMBOL = '?Remove@QuadTree@@QAEXPAVQuadTreeObject@@I@Z'
SOURCE = 'src/krusty2/broadphase/Quadtree.cpp'
PROBES = {0x004dd1b0: 'RemoveObject', 0x004dd0a0: 'node deleted'}


def _leaf_node():
    return dict(children=None, blocks=[], y=(0.0, 1.0))


def _ensure_path(rng, root, indices, n_objects, target):
    """Make the path exist below root and place `target` in the leaf; may break it off."""
    node = root
    for level, index in enumerate(indices):
        if rng.random() < 0.05:
            if node['children'] is not None:
                node['children'][index] = None
            return
        if node['children'] is None:
            node['children'] = [None] * 4
        if node['children'][index] is None:
            node['children'][index] = _leaf_node()
        node = node['children'][index]
    style = rng.randrange(5)
    if style == 0:                      # alone: the leaf empties
        node['blocks'] = [[target, None, None, None]]
        if rng.random() < 0.5:
            node['children'] = None
    elif style == 1:                    # next to others in one block
        block = [rng.randrange(n_objects) for _ in range(4)]
        block[rng.randrange(4)] = target
        node['blocks'] = [block]
    elif style == 2:                    # second block, first block full of others
        node['blocks'] = [[rng.randrange(n_objects) for _ in range(4)],
                          [None, target, None, None]]
    elif style == 3:                    # twice in the first block
        node['blocks'] = [[target, None, target, rng.choice([None, 0])]]
    else:                               # in an empty-slot block among others
        node['blocks'] = [[None, None, rng.randrange(n_objects), None], [target, None, None, None]]


def cases(rng, n):
    for _ in range(n):
        n_objects = rng.randrange(1, 6)
        target = n_objects                      # the object being removed (index n_objects)
        root = random_node(rng, 0, rng.randrange(0, MAX_DEPTH + 1), n_objects)
        depth = rng.randrange(0, MAX_DEPTH + 1)
        code = 0xf if rng.random() < 0.03 else make_code(rng, depth)
        if code != 0xf and rng.random() < 0.9:
            if root['children'] is None:
                root['children'] = [None] * 4
            for cx, cz in cell_origins(code):
                _ensure_path(rng, root, path_indices(cx, cz, depth), n_objects, target)
        yield (copy.deepcopy(root), n_objects, code, rng.random() < 0.2)


def setup(ctx, case):
    root, n_objects, code, other_tree = case
    objects = write_objects(ctx, n_objects + 1)
    tree = write_quadtree(ctx, root, objects, other_tree)
    return dict(args=[objects[n_objects], code], ecx=tree, ret_eax=False)  # void
