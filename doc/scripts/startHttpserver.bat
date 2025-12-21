@echo This batch file requires Node.js and the http-server package.
@echo Please install Node.js from https://nodejs.org/ and then run "npm install -g http-server" to install the http-server package.
@echo Starting HTTP server at port 8080...
cd %~dp0\..\html
http-server -p 8080