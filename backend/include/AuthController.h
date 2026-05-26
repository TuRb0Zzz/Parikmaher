#pragma once
#include <drogon/drogon.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;

class AuthController {
public:
    static Task<HttpResponsePtr> registerUser(const HttpRequestPtr req);
    static Task<HttpResponsePtr> loginUser(const HttpRequestPtr req);
    static Task<HttpResponsePtr> me(const HttpRequestPtr req);
    static Task<HttpResponsePtr> logout(const HttpRequestPtr req);
};