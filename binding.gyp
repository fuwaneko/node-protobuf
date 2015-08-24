{
	"targets": [{
		"target_name": "protobuf",
		"sources": [ "./src/init.cpp", "./src/native.cpp", "./src/parse.cpp", "./src/serialize.cpp" ],
		"cflags" : [ "-Ofast", "-mtune=native", "-ffast-math", "-funroll-loops", "-fomit-frame-pointer", "-std=c++11", "-pthread", "-static", "-I../../" ],
		"cflags_cc" : [ "-Ofast", "-mtune=native", "-ffast-math", "-funroll-loops", "-fomit-frame-pointer", "-std=c++11", "-pthread", "-static", "-I../../" ],
		"cflags_cc!": [ "-fno-rtti" ],
				"conditions": [
			["OS == 'win'", {
				"libraries": [
					"-llibprotobuf.lib"
				],
				"include_dirs": [
					"$(LIBPROTOBUF)/include",
					"<!(node -e \"require('nan')\")"
				],
				"msvs_settings": {
					"VCLinkerTool": {
						"AdditionalLibraryDirectories": "$(LIBPROTOBUF)/lib"
					}
				}
			}],
			["OS == 'mac'", {
				"libraries": [
					"-lprotobuf",
					"-L/usr/local/lib"
				],
				"include_dirs": [
					"<!(node -e \"require('nan')\")"
				],
				"xcode_settings": {
					"MACOSX_DEPLOYMENT_TARGET": "10.7",
					"OTHER_CPLUSPLUSFLAGS": [
						"-stdlib=libc++",
						"-I/usr/local/include"
					],
					"GCC_ENABLE_CPP_RTTI": "YES"
				}
			}],
			["OS == 'linux'", {
				"libraries": [
					"-L/usr/local/libz", 
					"-lprotobuf"
				],
				"include_dirs": [
					"<!(node -e \"require('nan')\")"
				]
			}]
		]
	}]
}
