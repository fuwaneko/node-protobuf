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
					"-lprotobuf"
				],
				"xcode_settings": {
					"MACOSX_DEPLOYMENT_TARGET": "10.7",
					"OTHER_CPLUSPLUSFLAGS": [
						"-stdlib=libc++"
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
