if(NOT DEFINED VS_PRODUCTS)
    set(VS_PRODUCTS
        "Microsoft.VisualStudio.Product.BuildTools"
        "Microsoft.VisualStudio.Product.Community"
        "Microsoft.VisualStudio.Product.Professional"
        "Microsoft.VisualStudio.Product.Enterprise"
    )
endif()

if(NOT DEFINED VS_WORKLOADS)
    set(VS_WORKLOADS
        "Microsoft.VisualStudio.Workload.VCTools"
    )
endif()

if(NOT DEFINED VS_COMPONENTS)
    set(VS_COMPONENTS
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
        "Microsoft.VisualStudio.Component.VC.CMake.Project"
        "Microsoft.VisualStudio.Component.VC.ATL"
        "Microsoft.VisualStudio.Component.VC.ATLMFC"
    )
endif()

if(NOT VS_VSWHERE_FULL_PATH)
    if(NOT VS_INSTALLER_PATHS)
        set(_PROGRAM_FILES "ProgramFiles")
        set(_PROGRAM_FILES_X86 "ProgramFiles(x86)")

        set(VS_INSTALLER_PATHS
            "$ENV{${_PROGRAM_FILES_X86}}/Microsoft Visual Studio/Installer"
            "$ENV{SystemDrive}/Program Files (x86)/Microsoft Visual Studio/Installer"
            "$ENV{${_PROGRAM_FILES}}/Microsoft Visual Studio/Installer"
            "$ENV{SystemDrive}/Program Files/Microsoft Visual Studio/Installer"
        )

        unset(_PROGRAM_FILES)
        unset(_PROGRAM_FILES_X86)
    endif()
    find_program(VS_VSWHERE_FULL_PATH
        NAMES "vswhere.exe"
        HINTS ${VS_INSTALLER_PATHS}
        NO_CACHE
    )
endif()

if(VS_VSWHERE_FULL_PATH)
    execute_process(COMMAND "${VS_VSWHERE_FULL_PATH}"
            -products ${VS_PRODUCTS}
            -requires ${VS_WORKLOADS} ${VS_COMPONENTS}
            -latest
            -format value
            -property installationPath
            -utf8
        OUTPUT_VARIABLE VS_INSTALLATION_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(VS_INSTALLATION_PATH)
        file(TO_CMAKE_PATH "${VS_INSTALLATION_PATH}" VS_INSTALLATION_PATH)
    endif()
    if(NOT EXISTS "${VS_INSTALLATION_PATH}")
        unset(VS_INSTALLATION_PATH)
    endif()
endif()

if(VS_INSTALLATION_PATH)
    message(STATUS "Found Visual Studio: ${VS_INSTALLATION_PATH}")
    set(VS_FOUND TRUE)
endif()
