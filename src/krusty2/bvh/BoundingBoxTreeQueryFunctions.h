#pragma once

// Query-tree ownership entry point, shared without importing the query TU's
// inline arithmetic helpers or per-TU vector constants into collision callers.
struct QueryTreeNode;
void FreeBoxTree(QueryTreeNode* node); // 0x0042a160, recursive postorder destruction
