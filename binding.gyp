{
	"targets": [{
		"target_name": "protobuf",
		"sources": [ "protobuf.cpp" ],
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
					]
				}
			}],
			["OS == 'linux'", {
				"libraries": [
					"-lprotobuf"
				]
			}]
		]
	}]
}
