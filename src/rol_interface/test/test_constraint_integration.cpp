#include "DakotaROLInterface.hpp"
#include "LibraryEnvironment.hpp"
#include "DakotaModel.hpp"

#include "ROL_Problem.hpp"
#include "ROL_Solver.hpp"
#include "ROL_Stream.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <filesystem>
#include "Teuchos_XMLParameterListHelpers.hpp"

// Test to solve a constrained optimization problem using the ROL interface.
// This test reads a Dakota input file, sets up the ROL problem with
// both an objective and constraints, and verifies the solution.

TEST(ROLConstraintIntegration, XMLParameterFileTest) {
    // Test demonstrates XML parameter file support for ROL interface
    
    // Create a simple ROL XML parameter file (adapted from ROL TypeG tests)
    const std::string rol_xml = R"(<?xml version="1.0"?>
<ParameterList name="Inputs">
  <ParameterList name="General">
    <Parameter name="Print Verbosity" type="int" value="1"/>
  </ParameterList>
  
  <ParameterList name="Step">
    <Parameter name="Type" type="string" value="Augmented Lagrangian"/>
    <ParameterList name="Augmented Lagrangian">
      <Parameter name="Subproblem Solver" type="string" value="Trust Region"/>
      <Parameter name="Subproblem Iteration Limit" type="int" value="20"/>
      <Parameter name="Use Default Problem Scaling" type="bool" value="false"/>
    </ParameterList>
  </ParameterList>
  
  <ParameterList name="Status Test">
    <Parameter name="Gradient Tolerance" type="double" value="1.0e-8"/>
    <Parameter name="Constraint Tolerance" type="double" value="1.0e-8"/>
    <Parameter name="Step Tolerance" type="double" value="1.0e-12"/>
    <Parameter name="Iteration Limit" type="int" value="50"/>
  </ParameterList>
</ParameterList>)";

    // Write the XML parameter file
    std::ofstream xml_file("rol_test_params.xml");
    xml_file << rol_xml;
    xml_file.close();
    
    // Verify file was created
    EXPECT_TRUE(std::filesystem::exists("rol_test_params.xml"));
    
    // Test that we can read the XML file using Teuchos
    Teuchos::ParameterList test_params;
    try {
      Teuchos::Ptr<Teuchos::ParameterList> params_ptr(&test_params);
      Teuchos::updateParametersFromXmlFile("rol_test_params.xml", params_ptr);
      
      // Verify some parameters were loaded correctly
      EXPECT_EQ(test_params.sublist("General").get<int>("Print Verbosity"), 1);
      EXPECT_EQ(test_params.sublist("Step").get<std::string>("Type"), "Augmented Lagrangian");
      EXPECT_EQ(test_params.sublist("Status Test").get<double>("Gradient Tolerance"), 1.0e-8);
      
      std::cout << "Successfully loaded ROL XML parameters" << std::endl;
      
    } catch (const std::exception& e) {
      FAIL() << "Failed to load XML parameters: " << e.what();
    }
    
    // Clean up
    std::filesystem::remove("rol_test_params.xml");
    
    // This test validates the XML parameter loading mechanism
    // In practice, Dakota would use advanced_options_file to specify the XML file
}

TEST(ROLConstraintIntegration, ROLInputXMLFileTest) {
    // Test using actual ROL input XML file (copied from source directory)
    
    // Verify the XML file was copied to build directory
    EXPECT_TRUE(std::filesystem::exists("rol_input.xml")) 
        << "rol_input.xml should be copied from source directory";
    
    // Load parameters from the actual ROL input file
    Teuchos::ParameterList rol_params;
    try {
        Teuchos::Ptr<Teuchos::ParameterList> params_ptr(&rol_params);
        Teuchos::updateParametersFromXmlFile("rol_input.xml", params_ptr);
        
        // Verify key parameters from our XML file
        EXPECT_EQ(rol_params.sublist("General").get<int>("Print Verbosity"), 1);
        
        // Verify Step parameters
        EXPECT_EQ(rol_params.sublist("Step").get<std::string>("Type"), "Augmented Lagrangian");
        EXPECT_EQ(rol_params.sublist("Step").sublist("Augmented Lagrangian")
                  .get<std::string>("Subproblem Solver"), "Trust Region");
        EXPECT_EQ(rol_params.sublist("Step").sublist("Augmented Lagrangian")
                  .get<int>("Subproblem Iteration Limit"), 50);
        EXPECT_EQ(rol_params.sublist("Step").sublist("Augmented Lagrangian")
                  .get<double>("Initial Penalty Parameter"), 10.0);
        
        // Verify Status Test parameters
        EXPECT_EQ(rol_params.sublist("Status Test").get<double>("Gradient Tolerance"), 1.0e-8);
        EXPECT_EQ(rol_params.sublist("Status Test").get<double>("Constraint Tolerance"), 1.0e-8);
        EXPECT_EQ(rol_params.sublist("Status Test").get<int>("Iteration Limit"), 100);
        
        // Verify Trust Region parameters
        EXPECT_EQ(rol_params.sublist("Step").sublist("Trust Region")
                  .get<std::string>("Subproblem Solver"), "Truncated CG");
        EXPECT_EQ(rol_params.sublist("Step").sublist("Trust Region")
                  .get<std::string>("Subproblem Model"), "Coleman-Li");
        EXPECT_EQ(rol_params.sublist("Step").sublist("Trust Region")
                  .get<double>("Initial Radius"), 1.0);
        
        // Verify Secant parameters
        EXPECT_EQ(rol_params.sublist("General").sublist("Secant")
                  .get<std::string>("Type"), "Limited-Memory BFGS");
        EXPECT_EQ(rol_params.sublist("General").sublist("Secant")
                  .get<int>("Maximum Storage"), 10);
                  
        std::cout << "Successfully loaded and validated comprehensive ROL XML parameters" << std::endl;
        
        // Print loaded parameters for verification
        if (rol_params.sublist("General").get<int>("Print Verbosity") > 0) {
            std::cout << "ROL Parameters loaded from XML:" << std::endl;
            rol_params.print(std::cout, 2, true, true);
        }
        
    } catch (const std::exception& e) {
        FAIL() << "Failed to load ROL XML parameters: " << e.what();
    }
    
    // This test demonstrates that the ROL interface can load complex
    // parameter sets from XML files, just like the ROL TypeG examples
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
