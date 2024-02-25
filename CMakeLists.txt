cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)
set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "" FORCE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#------------------------------------------------------------------------------
project(adaptagrams)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cola)

message ("Compiler ID : " ${CMAKE_CXX_COMPILER_ID})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    
    message ("Adding MFC...")
    
    add_definitions(-DNOMINMAX)
    add_definitions(-DLIBAVOID_NO_DLL)
    add_definitions(-D_WIN32_WINNT=0x0501)
    add_definitions(-D_USE_MATH_DEFINES)
#    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch)
endif()

#------------------------------------------------------------------------------
set(avoid_SOURCES
    ./cola/libavoid/vpsc.cpp
    ./cola/libavoid/visibility.cpp
    ./cola/libavoid/viscluster.cpp
    ./cola/libavoid/vertices.cpp
    ./cola/libavoid/timer.cpp
    ./cola/libavoid/shape.cpp
    ./cola/libavoid/scanline.cpp
    ./cola/libavoid/router.cpp
    ./cola/libavoid/orthogonal.cpp
    ./cola/libavoid/obstacle.cpp
    ./cola/libavoid/mtst.cpp
    ./cola/libavoid/makepath.cpp
    ./cola/libavoid/junction.cpp
    ./cola/libavoid/hyperedgetree.cpp
    ./cola/libavoid/hyperedgeimprover.cpp
    ./cola/libavoid/hyperedge.cpp
    ./cola/libavoid/graph.cpp
    ./cola/libavoid/geomtypes.cpp
    ./cola/libavoid/geometry.cpp
    ./cola/libavoid/connend.cpp
    ./cola/libavoid/connector.cpp
    ./cola/libavoid/connectionpin.cpp
    ./cola/libavoid/actioninfo.cpp

    ./cola/libavoid/vpsc.h
    ./cola/libavoid/visibility.h
    ./cola/libavoid/viscluster.h
    ./cola/libavoid/vertices.h
    ./cola/libavoid/timer.h
    ./cola/libavoid/shape.h
    ./cola/libavoid/scanline.h
    ./cola/libavoid/router.h
    ./cola/libavoid/orthogonal.h
    ./cola/libavoid/obstacle.h
    ./cola/libavoid/mtst.h
    ./cola/libavoid/makepath.h
    ./cola/libavoid/libavoid.h
    ./cola/libavoid/junction.h
    ./cola/libavoid/hyperedgetree.h
    ./cola/libavoid/hyperedgeimprover.h
    ./cola/libavoid/hyperedge.h
    ./cola/libavoid/graph.h
    ./cola/libavoid/geomtypes.h
    ./cola/libavoid/geometry.h
    ./cola/libavoid/dllexport.h
    ./cola/libavoid/debughandler.h
    ./cola/libavoid/debug.h
    ./cola/libavoid/connend.h
    ./cola/libavoid/connector.h
    ./cola/libavoid/connectionpin.h
    ./cola/libavoid/assertions.h
    ./cola/libavoid/actioninfo.h
    )

add_library(avoid STATIC ${avoid_SOURCES})



#------------------------------------------------------------------------------

set(cola_SOURCES 
    ./cola/libcola/output_svg.cpp
    ./cola/libcola/gradient_projection.cpp
    ./cola/libcola/convex_hull.cpp
    ./cola/libcola/connected_components.cpp
    ./cola/libcola/conjugate_gradient.cpp
    ./cola/libcola/compound_constraints.cpp
    ./cola/libcola/colafd.cpp
    ./cola/libcola/cola_log.cpp
    ./cola/libcola/cola.cpp
    ./cola/libcola/cluster.cpp
    ./cola/libcola/cc_nonoverlapconstraints.cpp
    ./cola/libcola/cc_clustercontainmentconstraints.cpp
    ./cola/libcola/box.cpp
    ./cola/libcola/unused.h
    ./cola/libcola/straightener.h
    ./cola/libcola/sparse_matrix.h
    ./cola/libcola/shortest_paths.h
    ./cola/libcola/shapepair.h
    ./cola/libcola/pseudorandom.h
    ./cola/libcola/output_svg.h
    ./cola/libcola/gradient_projection.h
    ./cola/libcola/exceptions.h
    ./cola/libcola/convex_hull.h
    ./cola/libcola/connected_components.h
    ./cola/libcola/conjugate_gradient.h
    ./cola/libcola/compound_constraints.h
    ./cola/libcola/commondefs.h
    ./cola/libcola/cola_log.h
    ./cola/libcola/cola.h
    ./cola/libcola/cluster.h
    ./cola/libcola/cc_nonoverlapconstraints.h
    ./cola/libcola/cc_clustercontainmentconstraints.h
    ./cola/libcola/box.h
    ./cola/libcola/straightener.cpp
    ./cola/libcola/shapepair.cpp
    ./cola/libcola/pseudorandom.cpp
    )

add_library(cola STATIC ${cola_SOURCES})
target_link_libraries(cola PUBLIC vpsc)





#------------------------------------------------------------------------------

set(dialect_SOURCES 
    ./cola/libdialect/expansion.cpp
    ./cola/libdialect/faces.cpp
    ./cola/libdialect/graphs.cpp
    ./cola/libdialect/hola.cpp
    ./cola/libdialect/io.cpp
    ./cola/libdialect/logging.cpp
    ./cola/libdialect/nearalign.cpp
    ./cola/libdialect/nexes.cpp
    ./cola/libdialect/nodeconfig.cpp
    ./cola/libdialect/nodes.cpp
    ./cola/libdialect/ortho.cpp
    ./cola/libdialect/peeling.cpp
    ./cola/libdialect/planarise.cpp
    ./cola/libdialect/qalookup.cpp
    ./cola/libdialect/quadaction.cpp
    ./cola/libdialect/routing.cpp
    ./cola/libdialect/sides.cpp
    ./cola/libdialect/treeplacement.cpp
    ./cola/libdialect/trees.cpp
    ./cola/libdialect/aca.h
    ./cola/libdialect/chains.h
    ./cola/libdialect/commontypes.h
    ./cola/libdialect/constraints.h
    ./cola/libdialect/expansion.h
    ./cola/libdialect/faces.h
    ./cola/libdialect/graphs.h
    ./cola/libdialect/hola.h
    ./cola/libdialect/io.h
    ./cola/libdialect/libdialect.h
    ./cola/libdialect/logging.h
    ./cola/libdialect/nearalign.h
    ./cola/libdialect/nodeconfig.h
    ./cola/libdialect/opts.h
    ./cola/libdialect/ortho.h
    ./cola/libdialect/peeling.h
    ./cola/libdialect/planarise.h
    ./cola/libdialect/quadaction.h
    ./cola/libdialect/routing.h
    ./cola/libdialect/treeplacement.h
    ./cola/libdialect/trees.h
    ./cola/libdialect/util.h
    ./cola/libdialect/aca.cpp
    ./cola/libdialect/bendseqlookup.cpp
    ./cola/libdialect/chains.cpp
    ./cola/libdialect/constraints.cpp
    ./cola/libdialect/edges.cpp
    )

add_library(dialect STATIC ${dialect_SOURCES})
set_target_properties(dialect PROPERTIES POSITION_INDEPENDENT_CODE ON)






#------------------------------------------------------------------------------
set(project_SOURCES 
    ./cola/libproject/variable.h
    ./cola/libproject/util.h
    ./cola/libproject/project_log.h
    ./cola/libproject/project.h
    ./cola/libproject/util.cpp
    ./cola/libproject/project.cpp
    )

add_library(project STATIC ${project_SOURCES})
set_target_properties(project PROPERTIES POSITION_INDEPENDENT_CODE ON)






#------------------------------------------------------------------------------
set(topology_SOURCES 
    ./cola/libtopology/topology_log.h
    ./cola/libtopology/topology_graph.h
    ./cola/libtopology/topology_constraints.h
    ./cola/libtopology/orthogonal_topology.h
    ./cola/libtopology/cola_topology_addon.h
    ./cola/libtopology/topology_graph.cpp
    ./cola/libtopology/topology_constraints_constructor.cpp
    ./cola/libtopology/topology_constraints.cpp
    ./cola/libtopology/resize.cpp
    ./cola/libtopology/orthogonal_topology.cpp
    ./cola/libtopology/compute_forces.cpp
    ./cola/libtopology/cola_topology_addon.cpp
    ./cola/libtopology/util.h
    )
   
add_library(topology STATIC ${topology_SOURCES})








#------------------------------------------------------------------------------
set(vpsc_SOURCES 
    ./cola/libvpsc/cbuffer.h
    ./cola/libvpsc/constraint.h
    ./cola/libvpsc/exceptions.h
    ./cola/libvpsc/linesegment.h
    ./cola/libvpsc/pairing_heap.h
    ./cola/libvpsc/rectangle.h
    ./cola/libvpsc/solve_VPSC.h
    ./cola/libvpsc/variable.h
    ./cola/libvpsc/block.cpp
    ./cola/libvpsc/blocks.cpp
    ./cola/libvpsc/cbuffer.cpp
    ./cola/libvpsc/constraint.cpp
    ./cola/libvpsc/rectangle.cpp
    ./cola/libvpsc/solve_VPSC.cpp
    ./cola/libvpsc/variable.cpp
    ./cola/libvpsc/assertions.h
    ./cola/libvpsc/block.h
    ./cola/libvpsc/blocks.h
    )

add_library(vpsc STATIC ${vpsc_SOURCES})

 