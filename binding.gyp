{
	"targets": [{
		"target_name": "protobuf",
		"sources": [ "./src/init.cpp", "./src/native.cpp", "./src/parse.cpp", "./src/serialize.cpp" ],
		"include_dirs": [
			"<!(node -e \"require('nan')\")"
		],
		"cflags" : [ "-Ofast", "-mtune=native", "-ffast-math", "-funroll-loops", "-fomit-frame-pointer", "-std=c++11", "-pthread", "-static", "-I../../" ],
		"cflags_cc" : [ "-Ofast", "-mtune=native", "-ffast-math", "-funroll-loops", "-fomit-frame-pointer", "-std=c++11", "-pthread", "-static", "-I../../" ],
		"conditions": [
			["OS == 'win'", {
				"libraries": [
					"-llibprotobuf.lib"
				],
				"include_dirs": [
					"$(LIBPROTOBUF)/include"
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
				"cflags_cc!": [ "-fno-rtti" ],
				"libraries": [
					"-L/usr/local/libz", 
					"-lprotobuf"
				]
			}]
		]
	}]
}
