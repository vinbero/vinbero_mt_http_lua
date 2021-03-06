# vinbero_mt_http_lua
[![GitHub release](http://img.shields.io/github/release/vinbero/vinbero_mt_http_lua.svg)](https://github.com/vinbero/vinbero_mt_http_lua/releases)
[![Github All Releases](http://img.shields.io/github/downloads/vinbero/vinbero_mt_http_lua/total.svg)](https://github.com/vinbero/vinbero_mt_http_lua/releases)
[![Build Status](https://travis-ci.org/vinbero/vinbero_mt_http_lua.svg?branch=master)](https://travis-ci.org/vinbero/vinbero_mt_http_lua)
[![license](http://img.shields.io/github/license/vinbero/vinbero_mt_http_lua.svg)](https://raw.githubusercontent.com/vinbero/vinbero_mt_http_lua/master/LICENSE)
[![Docker Stars](http://img.shields.io/docker/stars/vinbero/vinbero_mt_http_lua.svg)](https://hub.docker.com/r/vinbero/vinbero_mt_http_lua)
[![Docker Pulls](http://img.shields.io/docker/pulls/vinbero/vinbero_mt_http_lua.svg)](https://hub.docker.com/r/vinbero/vinbero_mt_http_lua)

A vinbero module for writing lua web application.

## Intoduction

## Interface implementations
- MODULE
- TLOCAL
- CLOCAL
- HTTP

## Config options
- vinbero_mt_http_lua.scriptFile(string) : Path of a lua script file. (relative/absolute)
- vinbero_mt_http_lua.scriptArg(string) : An argument to pass to the lua script file. You can get the value from vinbero.arg in the lua script.

## Globals in Lua script
- vinbero
- onInit()
- onRequestStart(client)
- onRequestHeadersFinish(client)
- onRequestBodyStart(client)
- onRequestBody(client, bodyChunk)
- onRequestBodyFinish(client)
- onRequestFinish(client)
- onDestroy()

## License
MPL-2.0
