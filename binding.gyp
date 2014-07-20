{
	"targets": [{
		"target_name": "protobuf",
		"sources": [ "./src/init.cpp", "./src/native.cpp", "./src/parse.cpp", "./src/serialize.cpp" ],
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
					]
				}
			}],
			["OS == 'linux'", {
				"libraries": [
					"-lprotobuf"
				],
				"include_dirs": [
					"<!(node -e \"require('nan')\")"
				]
			}]
		]
	}]
}
