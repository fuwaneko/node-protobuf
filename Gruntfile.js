/*global module:true*/
module.exports = function (grunt) {
	// Project configuration.
	grunt.initConfig({

		release: {
			options: {
				npm: false,
				tagName: "v<%= version %>",
				commitMessage: "Released v<%= version %>",
				tagMessage: "Tagged v<%= version %>"
			}
		}

	});

	grunt.loadNpmTasks("grunt-release");
};
