#include "../../src/reconstructed/ContainerList.h"

// Explicitly instantiate the pointer specialization used by the input-device
// classes. Retail folds its out-of-line members across those users.
struct UnknownControlBinding;
template class ContainerList<UnknownControlBinding*>;
