#pragma once

// When set to true, suppresses all std::cout logging from the server.
// Set this before calling Server::start() (or pass -q on the command line).
extern bool g_silent;
