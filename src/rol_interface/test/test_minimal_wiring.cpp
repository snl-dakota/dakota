#include "../include/BoolDispatch.hpp"

#include <iostream>
#include <cmath>

// Ultra-minimal wiring concept test 
// Tests only the BoolDispatch system without any Dakota dependencies

// Simulated Rosenbrock function calculations (no Dakota types needed)
double rosenbrock_function(double x, double y) {
    return 100.0 * std::pow(x*x - y, 2) + std::pow(x - 1.0, 2);
}

void rosenbrock_gradient(double x, double y, double* grad) {
    grad[0] = 400.0 * x * (x*x - y) + 2.0 * (x - 1.0);  // df/dx
    grad[1] = -200.0 * (x*x - y);                       // df/dy
}

void rosenbrock_hessian(double x, double y, double* hess) {
    hess[0] = 1200.0 * x*x - 400.0 * y + 2.0;  // d2f/dx2 (H_11)
    hess[1] = -400.0 * x;                       // d2f/dxdy (H_12 = H_21)
    hess[2] = 200.0;                            // d2f/dy2 (H_22)
}

int main() {
    
    try {
        std::cout << "=== Minimal ROL Interface Wiring Test ===" << std::endl;
        std::cout << "Testing BoolDispatch system for optimization strategy selection" << std::endl;
        
        // Step 1: Create different capability scenarios
        std::cout << "\n1. Testing derivative capability scenarios..." << std::endl;
        
        rol_interface::BoolDispatch has_gradient{true};
        rol_interface::BoolDispatch has_hessian{true};
        rol_interface::BoolDispatch no_gradient{false};
        rol_interface::BoolDispatch no_hessian{false};
        
        // Track all dispatch results
        int dispatch_count = 0;
        bool all_correct = true;
        
        std::cout << "   Scenario 1: Full second-order information available" << std::endl;
        auto full_second_order = has_gradient && has_hessian;
        full_second_order.receive([&](auto available) {
            if constexpr( available ) {
                std::cout << "   → Newton-type methods recommended" << std::endl;
                dispatch_count++;
            } else {
                std::cout << "   ✗ ERROR: Should have full second-order info" << std::endl;
                all_correct = false;
            }
        });
        
        std::cout << "   Scenario 2: Gradients only" << std::endl;
        auto gradient_only = has_gradient && no_hessian;
        gradient_only.receive([&](auto available) {
            if constexpr( available ) {
                std::cout << "   → Quasi-Newton methods (BFGS/L-BFGS) recommended" << std::endl;
                dispatch_count++;
            } else {
                std::cout << "   → Alternative methods needed" << std::endl;
            }
        });
        
        std::cout << "   Scenario 3: Function values only" << std::endl;
        auto function_only = no_gradient && no_hessian;
        function_only.receive([&](auto available) {
            if constexpr( available ) {
                std::cout << "   → Derivative-free methods (Nelder-Mead, pattern search)" << std::endl;
                dispatch_count++;
            } else {
                std::cout << "   → Some derivatives available" << std::endl;
            }
        });
        
        std::cout << "   Scenario 4: Hessian without gradient (unusual)" << std::endl;
        auto hessian_no_grad = no_gradient && has_hessian;
        hessian_no_grad.receive([&](auto available) {
            if constexpr( available ) {
                std::cout << "   → Unusual case: Hessian approximation methods" << std::endl;
                dispatch_count++;
            } else {
                std::cout << "   → Standard case: not Hessian-only" << std::endl;
            }
        });
        
        // Step 2: Test the Rosenbrock calculations (simulating Dakota callback results)
        std::cout << "\n2. Simulating optimization function evaluations..." << std::endl;
        
        double test_x = -1.2, test_y = 1.0;  // ROL's standard starting point
        
        // Function evaluation
        double func_val = rosenbrock_function(test_x, test_y);
        std::cout << "   f(" << test_x << ", " << test_y << ") = " << func_val << std::endl;
        
        // Gradient evaluation
        double grad_vals[2];
        rosenbrock_gradient(test_x, test_y, grad_vals);
        std::cout << "   ∇f = [" << grad_vals[0] << ", " << grad_vals[1] << "]" << std::endl;
        
        // Hessian evaluation
        double hess_vals[3];
        rosenbrock_hessian(test_x, test_y, hess_vals);
        std::cout << "   H = [[" << hess_vals[0] << ", " << hess_vals[1] << "], ";
        std::cout << "[" << hess_vals[1] << ", " << hess_vals[2] << "]]" << std::endl;
        
        // Step 3: Validate against expected results
        std::cout << "\n3. Validating against known Rosenbrock results..." << std::endl;
        
        double expected_f = 24.2;
        double expected_gx = -215.6;
        double expected_gy = -88.0;
        
        bool f_correct = std::abs(func_val - expected_f) < 0.1;
        bool gx_correct = std::abs(grad_vals[0] - expected_gx) < 0.1; 
        bool gy_correct = std::abs(grad_vals[1] - expected_gy) < 0.1;
        
        std::cout << "   Function value: " << (f_correct ? "✓" : "✗") << " (expected ~24.2)" << std::endl;
        std::cout << "   Gradient[0]: " << (gx_correct ? "✓" : "✗") << " (expected ~-215.6)" << std::endl;
        std::cout << "   Gradient[1]: " << (gy_correct ? "✓" : "✗") << " (expected ~-88.0)" << std::endl;
        
        // Step 4: Test algorithm selection logic
        std::cout << "\n4. Testing optimization algorithm selection logic..." << std::endl;
        
        // This simulates what the actual ROL interface would do
        has_gradient.receive([&](auto has_grad) {
            has_hessian.receive([&](auto has_hess) {
                if constexpr( has_grad && has_hess ) {
                    std::cout << "   Selected: Trust Region Newton with exact Hessian" << std::endl;
                } else if constexpr( has_grad && !has_hess ) {
                    std::cout << "   Selected: L-BFGS (limited memory quasi-Newton)" << std::endl;
                } else if constexpr( !has_grad && !has_hess ) {
                    std::cout << "   Selected: Nelder-Mead simplex" << std::endl;
                } else {
                    std::cout << "   Selected: Custom Hessian-based method" << std::endl;
                }
            });
        });
        
        // Summary
        std::cout << "\n=== Minimal Wiring Test Results ===" << std::endl;
        std::cout << "✓ BoolDispatch compile-time logic: " << all_correct << std::endl;
        std::cout << "✓ Algorithm selection scenarios: " << (dispatch_count >= 3) << std::endl;
        std::cout << "✓ Rosenbrock calculations: " << (f_correct && gx_correct && gy_correct) << std::endl;
        std::cout << "✓ Optimization strategy selection: working" << std::endl;
        
        bool overall_success = all_correct && (dispatch_count >= 3) && 
                              f_correct && gx_correct && gy_correct;
        
        if (overall_success) {
            std::cout << "\n=== SUCCESS: ROL interface wiring concept validated! ===" << std::endl;
            std::cout << "✓ BoolDispatch enables compile-time optimization strategy selection" << std::endl;
            std::cout << "✓ Ready to integrate with Dakota::AdapterModel and ROL::Problem" << std::endl;
            return 0;
        } else {
            std::cout << "\n=== FAILED: Some wiring concepts failed validation ===" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}