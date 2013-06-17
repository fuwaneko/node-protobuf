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
			}, { # on linux and mac
				"libraries": [
					"-llibprotobuf"
				]
			}],
		]
	}]
}