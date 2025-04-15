# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Definir variável para o caminho do FreeRTOS-Kernel
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel)

# Importar o SDK da Raspberry Pi Pico (deve ser antes do projeto)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(tracked_direction C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(tracked_direction 
            tracked_direction.c 
            src/send_data.c 
            src/wifi_connect.c
            src/joystick.c
            src/matrix_control.c)

pico_set_program_name(tracked_direction "tracked_direction")
pico_set_program_version(tracked_direction "0.1")

# Generate PIO header
pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(tracked_direction 0)
pico_enable_stdio_usb(tracked_direction 1)

# Add the standard library to the build
target_link_libraries(tracked_direction
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        hardware_adc
        hardware_gpio
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4)

# Add the standard include files to the build
target_include_directories(tracked_direction PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/src
)

# Add any user requested libraries
target_link_libraries(tracked_direction 
        pico_cyw43_arch_none
        )

pico_add_extra_outputs(tracked_direction)

