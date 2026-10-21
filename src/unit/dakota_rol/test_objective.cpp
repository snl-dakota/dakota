#include "DakotaROLInterface.hpp"

#include <iostream>

// Simple compilation test for the ROL interface

int main() {
    
    try {
        std::cout << "Testing ROL Interface Compilation" << std::endl;
        std::cout << "==================================" << std::endl;
        
        // Test BoolDispatch
        rol_interface::BoolDispatch test_true{true};
        rol_interface::BoolDispatch test_false{false};
        
        std::cout << "BoolDispatch test_true: " << static_cast<bool>(test_true) << std::endl;
        std::cout << "BoolDispatch test_false: " << static_cast<bool>(test_false) << std::endl;
        
        // Test BoolDispatch operations
        auto test_and = test_true && test_false;
        auto test_or = test_true || test_false;
        auto test_not = !test_true;
        
        std::cout << "test_true && test_false: " << static_cast<bool>(test_and) << std::endl;
        std::cout << "test_true || test_false: " << static_cast<bool>(test_or) << std::endl;
        std::cout << "!test_true: " << static_cast<bool>(test_not) << std::endl;
        
        // Test Vector creation
        auto vec1 = rol_interface::make_vector(5, true);
        auto vec2 = rol_interface::make_vector(5, false);
        
        std::cout << "Created vectors of dimension 5" << std::endl;
        std::cout << "vec1 dimension: " << vec1->dimension() << std::endl;
        std::cout << "vec2 dimension: " << vec2->dimension() << std::endl;
        
        // Test vector operations
        vec1->randomize(-1.0, 1.0);
        vec2->setScalar(2.0);
        
        std::cout << "vec1 norm after randomization: " << vec1->norm() << std::endl;
        std::cout << "vec2 norm after setting to 2.0: " << vec2->norm() << std::endl;
        
        // Test dot product
        double dot_result = vec1->dot(*vec2);
        std::cout << "vec1 dot vec2: " << dot_result << std::endl;
        
        // Test BoolDispatch receive functionality
        bool received_true = false;
        bool received_false = false;
        
        test_true.receive([&](auto is_true) {
            if constexpr( is_true ) {
                received_true = true;
                std::cout << "BoolDispatch correctly dispatched to true branch" << std::endl;
            } else {
                std::cout << "ERROR: BoolDispatch incorrectly dispatched to false branch" << std::endl;
            }
        });
        
        test_false.receive([&](auto is_true) {
            if constexpr( is_true ) {
                std::cout << "ERROR: BoolDispatch incorrectly dispatched to true branch" << std::endl;
            } else {
                received_false = true;
                std::cout << "BoolDispatch correctly dispatched to false branch" << std::endl;
            }
        });
        
        if (received_true && received_false) {
            std::cout << "\n=== SUCCESS: All compilation and basic tests passed! ===" << std::endl;
            return 0;
        } else {
            std::cout << "\n=== FAILED: BoolDispatch receive test failed ===" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}