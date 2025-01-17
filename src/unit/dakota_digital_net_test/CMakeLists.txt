include(DakotaUnitTest)

dakota_add_unit_test(NAME dakota_digital_net_test
    SOURCES digital_net.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS )
