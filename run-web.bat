@echo off
echo [*] Starting local HTTP server at http://localhost:8000
echo [*] Press Ctrl+C to stop the server

start http://localhost:8000

python -m http.server 8000 -d "%~dp0dist\web"