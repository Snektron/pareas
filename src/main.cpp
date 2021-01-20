#include <iostream>
#include <cstdlib>
#include <futhark-generated.h>

int main() {
    auto* config = futhark_context_config_new();
    std::cout << "oef" << std::endl;
    futhark_context_config_free(config);
    return EXIT_SUCCESS;
}
