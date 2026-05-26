#include "AuthController.h"
#include <iostream>
#include <json/json.h>
#include <drogon/drogon.h>
#include <drogon/utils/coroutine.h>
#include <coroutine>
#include <ctime>
#include <string>

using namespace drogon;
using namespace std;

static string getTokenFromCookie(const HttpRequestPtr req) {
    auto cookies = req->getCookies();
    auto it = cookies.find("auth_token");
    if (it != cookies.end()) return it->second;
    return "";
}

static Json::Value userToJson(const orm::Row& row) {
    Json::Value user;
    user["id"] = row["id"].as<int>();
    user["name"] = row["name"].as<string>();
    user["phone"] = row["phone"].as<string>();
    user["email"] = row["email"].as<string>();
    user["role"] = row["role"].as<string>();
    user["category"] = row["category"].as<string>();
    user["discount"] = row["discount"].as<int>();
    if (row["photo_url"].isNull()) {
        user["photo_url"] = Json::nullValue;
    } else {
        user["photo_url"] = row["photo_url"].as<string>();
    }
    return user;
}

Task<HttpResponsePtr> AuthController::registerUser(const HttpRequestPtr req) {
    auto json = req->getJsonObject();
    if (!json){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Bad Json";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        co_return response;
    }
    
    string name = (*json)["name"].asString();
    string phone = (*json)["phone"].asString();
    string email = (*json)["email"].asString();
    string password = (*json)["password"].asString();
    
    if (name.empty() || phone.empty() || email.empty() || password.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Missing fields";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        co_return response;
    }
    
    auto db = app().getDbClient("default");
    try {
        auto existing = co_await db->execSqlCoro("SELECT 1 FROM users WHERE email = $1", email);
        if (!existing.empty()){
            Json::Value bad_answer;
            bad_answer["status"]="bad";
            bad_answer["message"]="Email already registered";
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k409Conflict);
            co_return response;
        }
        
        string passwordHash = utils::getSha256(password);
        
        auto result = co_await db->execSqlCoro(
            "INSERT INTO users (name, phone, email, password_hash) "
            "VALUES ($1, $2, $3, $4) "
            "RETURNING id, name, phone, email, role, category, discount, photo_url",
            name, phone, email, passwordHash);
        
        string token = utils::getUuid();
        co_await db->execSqlCoro(
            "INSERT INTO user_sessions (user_id, token, expires_at) VALUES ($1, $2, NOW() + INTERVAL '30 days')",
            result[0]["id"].as<int>(), token);
        
        Json::Value respJson;
        respJson["status"]="ok";
        respJson["user"] = userToJson(result[0]);
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k201Created);
        
        Cookie cookie("auth_token", token);
        cookie.setHttpOnly(true);
        cookie.setPath("/");
        cookie.setMaxAge(30*24*3600);
        resp->addCookie(cookie);
        co_return resp;
        
    } catch (const exception& e) {
        LOG_ERROR << "registerUser error: " << e.what();
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Database error";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        co_return response;
    }
}

Task<HttpResponsePtr> AuthController::loginUser(const HttpRequestPtr req) {
    auto json = req->getJsonObject();
    if (!json){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Bad Json";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        co_return response;
    }
    
    string email = (*json)["email"].asString();
    string password = (*json)["password"].asString();
    
    if (email.empty() || password.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Email and password required";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k400BadRequest);
        co_return response;
    }
    
    auto db = app().getDbClient("default");
    try {
        auto result = co_await db->execSqlCoro(
            "SELECT id, name, phone, email, password_hash, role, category, discount, photo_url "
            "FROM users WHERE email = $1", email);
        if (result.empty()){
            Json::Value bad_answer;
            bad_answer["status"]="bad";
            bad_answer["message"]="Invalid email or password";
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k401Unauthorized);
            co_return response;
        }
        
        string passwordHash = utils::getSha256(password);
        if (passwordHash != result[0]["password_hash"].as<string>()){
            Json::Value bad_answer;
            bad_answer["status"]="bad";
            bad_answer["message"]="Invalid email or password";
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k401Unauthorized);
            co_return response;
        }
        
        string token = utils::getUuid();
        co_await db->execSqlCoro(
            "INSERT INTO user_sessions (user_id, token, expires_at) VALUES ($1, $2, NOW() + INTERVAL '30 days') "
            "ON CONFLICT (user_id) DO UPDATE SET token = $2, expires_at = NOW() + INTERVAL '30 days'",
            result[0]["id"].as<int>(), token);
        
        Json::Value respJson;
        respJson["status"]="ok";
        respJson["user"] = userToJson(result[0]);
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        
        Cookie cookie("auth_token", token);
        cookie.setHttpOnly(true);
        cookie.setPath("/");
        cookie.setMaxAge(30*24*3600);
        resp->addCookie(cookie);
        co_return resp;
        
    } catch (const exception& e) {
        LOG_ERROR << "loginUser error: " << e.what();
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Database error";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        co_return response;
    }
}

Task<HttpResponsePtr> AuthController::me(const HttpRequestPtr req) {
    string token = getTokenFromCookie(req);
    if (token.empty()){
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="No auth token";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k401Unauthorized);
        co_return response;
    }
    
    auto db = app().getDbClient("default");
    try {
        auto result = co_await db->execSqlCoro(
            "SELECT u.id, u.name, u.phone, u.email, u.role, u.category, u.discount, u.photo_url "
            "FROM users u JOIN user_sessions s ON u.id = s.user_id "
            "WHERE s.token = $1 AND s.expires_at > NOW()", token);
        if (result.empty()){
            Json::Value bad_answer;
            bad_answer["status"]="bad";
            bad_answer["message"]="Invalid or expired token";
            auto response = HttpResponse::newHttpJsonResponse(bad_answer);
            response->setStatusCode(k401Unauthorized);
            co_return response;
        }
        
        Json::Value respJson;
        respJson["status"]="ok";
        respJson["user"] = userToJson(result[0]);
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
        
    } catch (const exception& e) {
        LOG_ERROR << "me error: " << e.what();
        Json::Value bad_answer;
        bad_answer["status"]="bad";
        bad_answer["message"]="Database error";
        auto response = HttpResponse::newHttpJsonResponse(bad_answer);
        response->setStatusCode(k500InternalServerError);
        co_return response;
    }
}

Task<HttpResponsePtr> AuthController::logout(const HttpRequestPtr req) {
    string token = getTokenFromCookie(req);
    auto db = app().getDbClient("default");
    if (!token.empty()){
        try {
            co_await db->execSqlCoro("DELETE FROM user_sessions WHERE token = $1", token);
        } catch (...) {}
    }
    
    Cookie clearCookie("auth_token", "");
    clearCookie.setHttpOnly(true);
    clearCookie.setPath("/");
    clearCookie.setMaxAge(0);
    
    Json::Value respJson;
    respJson["status"]="ok";
    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(k200OK);
    resp->addCookie(clearCookie);
    co_return resp;
}