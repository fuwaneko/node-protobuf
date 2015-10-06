/*global module:true*/
module.exports = function(grunt) {
	// Project configuration.
	grunt.initConfig({

		release: {
			options: {
				npm: true,
				tagName: "v<%= version %>",
				commitMessage: "Released v<%= version %>",
				tagMessage: "Tagged v<%= version %>",
				github: {
					repo: "fuwaneko/node-protobuf",
					accessTokenVar: "GH_ACCESS_TOKEN"
				}
			}
		}

	});

	grunt.loadNpmTasks("grunt-release");
};
