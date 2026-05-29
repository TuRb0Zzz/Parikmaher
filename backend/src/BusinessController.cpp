#include "BusinessController.h"
#include <json/json.h>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sys/stat.h>
#include <drogon/utils/Utilities.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace drogon;
using namespace std;

static string getTokenFromCookie(const HttpRequestPtr req) {
    auto cookies = req->getCookies();
    auto it = cookies.find("auth_token");
    if (it != cookies.end()) return it->second;
    return "";
}

static Task<pair<bool, int>> checkAdmin(const HttpRequestPtr req) {
    string token = getTokenFromCookie(req);
    if (token.empty()){
        co_return make_pair(false, 0);
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro("SELECT u.id, u.role FROM users u JOIN user_sessions s ON u.id = s.user_id WHERE s.token = $1 AND s.expires_at > NOW()", token);
    if (result.empty()){
        co_return make_pair(false, 0);
    }
    bool isAdmin = (result[0]["role"].as<string>() == "admin");
    co_return make_pair(isAdmin, result[0]["id"].as<int>());
}

static Task<int> getUserIdFromToken(const HttpRequestPtr req) {
    string token = getTokenFromCookie(req);
    if (token.empty()) co_return 0;
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT u.id FROM users u JOIN user_sessions s ON u.id = s.user_id "
        "WHERE s.token = $1 AND s.expires_at > NOW()", token);
    if (result.empty()) co_return 0;
    co_return result[0]["id"].as<int>();
}

static int getIntFromJson(const Json::Value& val, int defaultValue = 0) {
    if (val.isInt()) return val.asInt();
    if (val.isString()) {
        try { return stoi(val.asString()); } catch (...) { return defaultValue; }
    }
    return defaultValue;
}

static void ensureDir(const string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        mkdir(path.c_str(), 0755);
    }
}

static Json::Value formatPhotoUrl(const string& filename) {
    if (filename.empty()) return Json::nullValue;
    return Json::Value("http://localhost:8080/" + filename);
}

// ==================== Публичные ====================

Task<HttpResponsePtr> BusinessController::getServices(const HttpRequestPtr req) {
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro("SELECT id, name, gender, price, discount, photo_url FROM services");
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["name"] = row["name"].as<string>();
        item["gender"] = row["gender"].as<string>();
        item["price"] = row["price"].as<int>();
        item["discount"] = row["discount"].as<int>();
        string filename = row["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::getMasters(const HttpRequestPtr req) {
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro("SELECT id, name, specialization, rank, photo_url FROM masters");
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["name"] = row["name"].as<string>();
        item["specialization"] = row["specialization"].as<string>();
        item["rank"] = row["rank"].as<string>();
        string filename = row["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

// ==================== Пользовательские ====================

Task<HttpResponsePtr> BusinessController::createBooking(const HttpRequestPtr req) {
    int userId = co_await getUserIdFromToken(req);
    if (userId == 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Unauthorized";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k401Unauthorized);
        co_return resp;
    }
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    int serviceId = getIntFromJson((*json)["serviceId"], 0);
    int masterId = getIntFromJson((*json)["masterId"], 0);
    string dateStr = (*json)["date"].asString();
    if (serviceId == 0 || masterId == 0 || dateStr.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing fields";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        auto result = co_await db->execSqlCoro(
            "INSERT INTO bookings (user_id, service_id, master_id, booking_date, status) "
            "VALUES ($1, $2, $3, $4, 'pending') RETURNING id",
            userId, serviceId, masterId, dateStr);
        Json::Value respJson;
        respJson["status"]="ok";
        respJson["bookingId"] = result[0]["id"].as<int>();
        respJson["message"] = "Booking created";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k201Created);
        co_return resp;
    } catch (const exception& e) {
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::getUserBookings(const HttpRequestPtr req) {
    int userId = co_await getUserIdFromToken(req);
    if (userId == 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Unauthorized";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k401Unauthorized);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT b.id, s.name AS service_name, m.name AS master_name, b.booking_date, b.status "
        "FROM bookings b JOIN services s ON b.service_id = s.id "
        "JOIN masters m ON b.master_id = m.id "
        "WHERE b.user_id = $1 ORDER BY b.booking_date", userId);
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["service_name"] = row["service_name"].as<string>();
        item["master_name"] = row["master_name"].as<string>();
        item["date"] = row["booking_date"].as<string>();
        item["status"] = row["status"].as<string>();
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::cancelBooking(const HttpRequestPtr req, int bookingId) {
    int userId = co_await getUserIdFromToken(req);
    if (userId == 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Unauthorized";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k401Unauthorized);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        auto check = co_await db->execSqlCoro(
            "SELECT id FROM bookings WHERE id = $1 AND user_id = $2 AND status = 'pending'",
            bookingId, userId);
        if (check.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Booking not found or already confirmed/cancelled";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }
        co_await db->execSqlCoro("UPDATE bookings SET status = 'cancelled' WHERE id = $1", bookingId);
        Json::Value respJson; respJson["status"]="ok"; respJson["message"]="Booking cancelled";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::rescheduleBooking(const HttpRequestPtr req, int bookingId) {
    int userId = co_await getUserIdFromToken(req);
    if (userId == 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Unauthorized";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k401Unauthorized);
        co_return resp;
    }
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    string newDate = (*json)["date"].asString();
    if (newDate.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing date";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        auto check = co_await db->execSqlCoro(
            "SELECT id FROM bookings WHERE id = $1 AND user_id = $2 AND status = 'pending'",
            bookingId, userId);
        if (check.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Booking not found or not pending";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }
        co_await db->execSqlCoro("UPDATE bookings SET booking_date = $1 WHERE id = $2", newDate, bookingId);
        Json::Value respJson; respJson["status"]="ok"; respJson["message"]="Booking rescheduled";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

// ==================== Админ: клиенты ====================

Task<HttpResponsePtr> BusinessController::adminGetClients(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro("SELECT id, name, category, gender, service, date FROM clients");
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["name"] = row["name"].as<string>();
        item["category"] = row["category"].as<string>();
        item["gender"] = row["gender"].as<string>();
        item["service"] = row["service"].as<string>();
        item["date"] = row["date"].as<string>();
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::adminCreateClient(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    string name = (*json)["name"].asString();
    string category = (*json)["category"].asString();
    string gender = (*json)["gender"].asString();
    string service = (*json)["service"].asString();
    string date = (*json)["date"].asString();
    if (name.empty() || category.empty() || gender.empty() || service.empty() || date.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing fields";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        auto result = co_await db->execSqlCoro(
            "INSERT INTO clients (name, category, gender, service, date) VALUES ($1, $2, $3, $4, $5) RETURNING id, name, category, gender, service, date",
            name, category, gender, service, date);
        Json::Value item;
        item["id"] = result[0]["id"].as<int>();
        item["name"] = result[0]["name"].as<string>();
        item["category"] = result[0]["category"].as<string>();
        item["gender"] = result[0]["gender"].as<string>();
        item["service"] = result[0]["service"].as<string>();
        item["date"] = result[0]["date"].as<string>();
        auto resp = HttpResponse::newHttpJsonResponse(item);
        resp->setStatusCode(k201Created);
        co_return resp;
    } catch (const exception& e) {
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::adminDeleteClient(const HttpRequestPtr req, int id) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        co_await db->execSqlCoro("DELETE FROM clients WHERE id = $1", id);
        Json::Value respJson; respJson["status"]="ok"; respJson["message"]="Client deleted";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::adminUpdateClient(const HttpRequestPtr req, int id) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        auto check = co_await db->execSqlCoro("SELECT id FROM clients WHERE id = $1", id);
        if (check.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Client not found";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }
        if ((*json).isMember("name")) {
            string val = (*json)["name"].asString();
            co_await db->execSqlCoro("UPDATE clients SET name = $1 WHERE id = $2", val, id);
        }
        if ((*json).isMember("category")) {
            string val = (*json)["category"].asString();
            co_await db->execSqlCoro("UPDATE clients SET category = $1 WHERE id = $2", val, id);
        }
        if ((*json).isMember("gender")) {
            string val = (*json)["gender"].asString();
            co_await db->execSqlCoro("UPDATE clients SET gender = $1 WHERE id = $2", val, id);
        }
        if ((*json).isMember("service")) {
            string val = (*json)["service"].asString();
            co_await db->execSqlCoro("UPDATE clients SET service = $1 WHERE id = $2", val, id);
        }
        if ((*json).isMember("date")) {
            string val = (*json)["date"].asString();
            co_await db->execSqlCoro("UPDATE clients SET date = $1 WHERE id = $2", val, id);
        }
        auto result = co_await db->execSqlCoro("SELECT id, name, category, gender, service, date FROM clients WHERE id = $1", id);
        if (result.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Client not found after update";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }
        Json::Value item;
        item["id"] = result[0]["id"].as<int>();
        item["name"] = result[0]["name"].as<string>();
        item["category"] = result[0]["category"].as<string>();
        item["gender"] = result[0]["gender"].as<string>();
        item["service"] = result[0]["service"].as<string>();
        item["date"] = result[0]["date"].as<string>();
        auto resp = HttpResponse::newHttpJsonResponse(item);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

// ==================== Админ: мастера ====================

Task<HttpResponsePtr> BusinessController::adminGetMasters(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro("SELECT id, name, specialization, rank, photo_url FROM masters");
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["name"] = row["name"].as<string>();
        item["specialization"] = row["specialization"].as<string>();
        item["rank"] = row["rank"].as<string>();
        string filename = row["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::adminCreateMaster(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    MultiPartParser parser;
    if (parser.parse(req) != 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid multipart data";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto &params = parser.getParameters();
    auto &files = parser.getFiles();
    string name, specialization, rank;
    auto it = params.find("name");
    if (it != params.end()) name = it->second;
    it = params.find("specialization");
    if (it != params.end()) specialization = it->second;
    it = params.find("rank");
    if (it != params.end()) rank = it->second;
    if (name.empty() || specialization.empty() || rank.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing fields (name, specialization, rank)";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    string photoName;
    for (auto &file : files) {
        if (file.getItemName() == "photo") {
            string ext = string(file.getFileExtension());
            string newName = utils::getUuid() + (ext.empty() ? ".jpg" : "." + ext);
            ensureDir("./images");
            string savePath = "./images/" + newName;
            file.saveAs(savePath);
            photoName = newName;
            break;
        }
    }
    auto db = app().getDbClient("default");
    try {
        auto result = co_await db->execSqlCoro(
            "INSERT INTO masters (name, specialization, rank, photo_url) VALUES ($1, $2, $3, $4) RETURNING id, name, specialization, rank, photo_url",
            name, specialization, rank, photoName);
        Json::Value item;
        item["id"] = result[0]["id"].as<int>();
        item["name"] = result[0]["name"].as<string>();
        item["specialization"] = result[0]["specialization"].as<string>();
        item["rank"] = result[0]["rank"].as<string>();
        string filename = result[0]["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        auto resp = HttpResponse::newHttpJsonResponse(item);
        resp->setStatusCode(k201Created);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminCreateMaster error: " << e.what();
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::adminDeleteMaster(const HttpRequestPtr req, int id) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        // Проверяем, есть ли связанные бронирования
        auto bookingCheck = co_await db->execSqlCoro(
            "SELECT id FROM bookings WHERE master_id = $1 LIMIT 1", id);
        if (!bookingCheck.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Cannot delete master: there are existing bookings for this master";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        // Получаем photo_url перед удалением
        auto res = co_await db->execSqlCoro("SELECT photo_url FROM masters WHERE id = $1", id);
        if (!res.empty() && !res[0]["photo_url"].isNull()) {
            string filename = res[0]["photo_url"].as<string>();
            if (!filename.empty()) {
                string filePath = "./images/" + filename;
                remove(filePath.c_str());
            }
        }
        co_await db->execSqlCoro("DELETE FROM masters WHERE id = $1", id);
        Json::Value respJson; respJson["status"]="ok"; respJson["message"]="Master deleted";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminDeleteMaster error: " << e.what();
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::adminUpdateMaster(const HttpRequestPtr req, int id) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    MultiPartParser parser;
    if (parser.parse(req) != 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid multipart data";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto &params = parser.getParameters();
    auto &files = parser.getFiles();
    auto db = app().getDbClient("default");
    try {
        auto check = co_await db->execSqlCoro("SELECT id, photo_url FROM masters WHERE id = $1", id);
        if (check.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Master not found";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }
        string oldFilename = check[0]["photo_url"].isNull() ? "" : check[0]["photo_url"].as<string>();
        auto it = params.find("name");
        if (it != params.end() && !it->second.empty()) {
            co_await db->execSqlCoro("UPDATE masters SET name = $1 WHERE id = $2", it->second, id);
        }
        it = params.find("specialization");
        if (it != params.end() && !it->second.empty()) {
            co_await db->execSqlCoro("UPDATE masters SET specialization = $1 WHERE id = $2", it->second, id);
        }
        it = params.find("rank");
        if (it != params.end() && !it->second.empty()) {
            co_await db->execSqlCoro("UPDATE masters SET rank = $1 WHERE id = $2", it->second, id);
        }
        string newFilename = oldFilename;
        for (auto &file : files) {
            if (file.getItemName() == "photo") {
                if (!oldFilename.empty()) {
                    string oldPath = "./images/" + oldFilename;
                    remove(oldPath.c_str());
                }
                string ext = string(file.getFileExtension());
                string newName = utils::getUuid() + (ext.empty() ? ".jpg" : "." + ext);
                ensureDir("./images");
                string savePath = "./images/" + newName;
                file.saveAs(savePath);
                newFilename = newName;
                co_await db->execSqlCoro("UPDATE masters SET photo_url = $1 WHERE id = $2", newFilename, id);
                break;
            }
        }
        auto result = co_await db->execSqlCoro(
            "SELECT id, name, specialization, rank, photo_url FROM masters WHERE id = $1", id);
        Json::Value item;
        item["id"] = result[0]["id"].as<int>();
        item["name"] = result[0]["name"].as<string>();
        item["specialization"] = result[0]["specialization"].as<string>();
        item["rank"] = result[0]["rank"].as<string>();
        string filename = result[0]["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        auto resp = HttpResponse::newHttpJsonResponse(item);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminUpdateMaster error: " << e.what();
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

// ==================== Админ: услуги ====================

Task<HttpResponsePtr> BusinessController::adminGetServices(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro("SELECT id, name, gender, price, discount, photo_url FROM services");
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["name"] = row["name"].as<string>();
        item["gender"] = row["gender"].as<string>();
        item["price"] = row["price"].as<int>();
        item["discount"] = row["discount"].as<int>();
        string filename = row["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::adminCreateService(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    MultiPartParser parser;
    if (parser.parse(req) != 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid multipart data";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto &params = parser.getParameters();
    auto &files = parser.getFiles();
    string name, gender, priceStr, discountStr;
    auto it = params.find("name");
    if (it != params.end()) name = it->second;
    it = params.find("gender");
    if (it != params.end()) gender = it->second;
    it = params.find("price");
    if (it != params.end()) priceStr = it->second;
    it = params.find("discount");
    if (it != params.end()) discountStr = it->second;
    int price = 0, discount = 0;
    try { price = stoi(priceStr); } catch(...) {}
    try { discount = stoi(discountStr); } catch(...) {}
    if (name.empty() || gender.empty() || price <= 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing or invalid fields (name, gender, price)";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    string photoName;
    for (auto &file : files) {
        if (file.getItemName() == "photo") {
            string ext = string(file.getFileExtension());
            string newName = utils::getUuid() + (ext.empty() ? ".jpg" : "." + ext);
            ensureDir("./images");
            string savePath = "./images/" + newName;
            file.saveAs(savePath);
            photoName = newName;
            break;
        }
    }
    auto db = app().getDbClient("default");
    try {
        auto result = co_await db->execSqlCoro(
            "INSERT INTO services (name, gender, price, discount, photo_url) VALUES ($1, $2, $3, $4, $5) RETURNING id, name, gender, price, discount, photo_url",
            name, gender, price, discount, photoName);
        Json::Value item;
        item["id"] = result[0]["id"].as<int>();
        item["name"] = result[0]["name"].as<string>();
        item["gender"] = result[0]["gender"].as<string>();
        item["price"] = result[0]["price"].as<int>();
        item["discount"] = result[0]["discount"].as<int>();
        string filename = result[0]["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        auto resp = HttpResponse::newHttpJsonResponse(item);
        resp->setStatusCode(k201Created);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminCreateService error: " << e.what();
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::adminDeleteService(const HttpRequestPtr req, int id) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        // Проверяем, есть ли связанные бронирования
        auto bookingCheck = co_await db->execSqlCoro(
            "SELECT id FROM bookings WHERE service_id = $1 LIMIT 1", id);
        if (!bookingCheck.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Cannot delete service: there are existing bookings for this service";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k400BadRequest);
            co_return resp;
        }

        auto res = co_await db->execSqlCoro("SELECT photo_url FROM services WHERE id = $1", id);
        if (!res.empty() && !res[0]["photo_url"].isNull()) {
            string filename = res[0]["photo_url"].as<string>();
            if (!filename.empty()) {
                string filePath = "./images/" + filename;
                remove(filePath.c_str());
            }
        }
        co_await db->execSqlCoro("DELETE FROM services WHERE id = $1", id);
        Json::Value respJson; respJson["status"]="ok"; respJson["message"]="Service deleted";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminDeleteService error: " << e.what();
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

Task<HttpResponsePtr> BusinessController::adminUpdateService(const HttpRequestPtr req, int id) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    MultiPartParser parser;
    if (parser.parse(req) != 0) {
        Json::Value err; err["status"]="bad"; err["message"]="Invalid multipart data";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto &params = parser.getParameters();
    auto &files = parser.getFiles();
    auto db = app().getDbClient("default");
    try {
        auto check = co_await db->execSqlCoro("SELECT id, photo_url FROM services WHERE id = $1", id);
        if (check.empty()) {
            Json::Value err; err["status"]="bad"; err["message"]="Service not found";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }
        string oldFilename = check[0]["photo_url"].isNull() ? "" : check[0]["photo_url"].as<string>();
        auto it = params.find("name");
        if (it != params.end() && !it->second.empty()) {
            co_await db->execSqlCoro("UPDATE services SET name = $1 WHERE id = $2", it->second, id);
        }
        it = params.find("gender");
        if (it != params.end() && !it->second.empty()) {
            co_await db->execSqlCoro("UPDATE services SET gender = $1 WHERE id = $2", it->second, id);
        }
        it = params.find("price");
        if (it != params.end() && !it->second.empty()) {
            try {
                int price = stoi(it->second);
                co_await db->execSqlCoro("UPDATE services SET price = $1 WHERE id = $2", price, id);
            } catch (...) {
                LOG_ERROR << "Invalid price value: " << it->second;
            }
        }
        it = params.find("discount");
        if (it != params.end() && !it->second.empty()) {
            try {
                int discount = stoi(it->second);
                co_await db->execSqlCoro("UPDATE services SET discount = $1 WHERE id = $2", discount, id);
            } catch (...) {
                LOG_ERROR << "Invalid discount value: " << it->second;
            }
        }
        string newFilename = oldFilename;
        for (auto &file : files) {
            if (file.getItemName() == "photo") {
                if (!oldFilename.empty()) {
                    string oldPath = "./images/" + oldFilename;
                    remove(oldPath.c_str());
                }
                string ext = string(file.getFileExtension());
                string newName = utils::getUuid() + (ext.empty() ? ".jpg" : "." + ext);
                ensureDir("./images");
                string savePath = "./images/" + newName;
                file.saveAs(savePath);
                newFilename = newName;
                co_await db->execSqlCoro("UPDATE services SET photo_url = $1 WHERE id = $2", newFilename, id);
                break;
            }
        }
        auto result = co_await db->execSqlCoro(
            "SELECT id, name, gender, price, discount, photo_url FROM services WHERE id = $1", id);
        Json::Value item;
        item["id"] = result[0]["id"].as<int>();
        item["name"] = result[0]["name"].as<string>();
        item["gender"] = result[0]["gender"].as<string>();
        item["price"] = result[0]["price"].as<int>();
        item["discount"] = result[0]["discount"].as<int>();
        string filename = result[0]["photo_url"].as<string>();
        item["photo_url"] = formatPhotoUrl(filename);
        auto resp = HttpResponse::newHttpJsonResponse(item);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminUpdateService error: " << e.what();
        Json::Value err; err["status"]="bad"; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

// ==================== Админ: бронирования (включая клиентов) ====================

Task<HttpResponsePtr> BusinessController::adminGetAllBookings(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT b.id, u.name AS user_name, s.name AS service_name, m.name AS master_name, b.booking_date, b.status "
        "FROM bookings b JOIN users u ON b.user_id = u.id "
        "JOIN services s ON b.service_id = s.id "
        "JOIN masters m ON b.master_id = m.id "
        "ORDER BY b.booking_date DESC");
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["user_name"] = row["user_name"].as<string>();
        item["service_name"] = row["service_name"].as<string>();
        item["master_name"] = row["master_name"].as<string>();
        item["date"] = row["booking_date"].as<string>();
        item["status"] = row["status"].as<string>();
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::adminConfirmBooking(const HttpRequestPtr req, int bookingId) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    try {
        auto check = co_await db->execSqlCoro(
            "SELECT id FROM bookings WHERE id = $1 AND status = 'pending'", bookingId);
        if (check.empty()) {
            Json::Value err; err["message"]="Booking not found or already confirmed/cancelled";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound);
            co_return resp;
        }

        co_await db->execSqlCoro("UPDATE bookings SET status = 'confirmed' WHERE id = $1", bookingId);

        // Получаем данные для вставки в clients
        auto data = co_await db->execSqlCoro(
            "SELECT u.name AS user_name, u.category AS user_category, "
            "       s.name AS service_name, s.gender AS service_gender, "
            "       DATE(b.booking_date) AS booking_date "
            "FROM bookings b "
            "JOIN users u ON b.user_id = u.id "
            "JOIN services s ON b.service_id = s.id "
            "WHERE b.id = $1", bookingId);
        if (!data.empty()) {
            string userName = data[0]["user_name"].as<string>();
            string userCategory = data[0]["user_category"].as<string>();
            string serviceName = data[0]["service_name"].as<string>();
            string serviceGender = data[0]["service_gender"].as<string>();
            string bookingDate = data[0]["booking_date"].as<string>();

            // Проверяем, не добавлен ли уже клиент (по имени, услуге и дате)
            auto existing = co_await db->execSqlCoro(
                "SELECT id FROM clients WHERE name = $1 AND service = $2 AND date = $3",
                userName, serviceName, bookingDate);
            if (existing.empty()) {
                co_await db->execSqlCoro(
                    "INSERT INTO clients (name, category, gender, service, date) "
                    "VALUES ($1, $2, $3, $4, $5)",
                    userName, userCategory, serviceGender, serviceName, bookingDate);
            }
        }

        Json::Value respJson; respJson["status"]="ok"; respJson["message"]="Booking confirmed";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(k200OK);
        co_return resp;
    } catch (const exception& e) {
        LOG_ERROR << "adminConfirmBooking error: " << e.what();
        Json::Value err; err["message"]="Database error";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError);
        co_return resp;
    }
}

// ==================== Отчёты ====================

Task<HttpResponsePtr> BusinessController::reportClientsByDate(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    string date = req->getParameter("date");
    if (date.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing date";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT id, name, category, gender, service, date FROM clients WHERE date = $1", date);
    Json::Value arr;
    for (auto& row : result) {
        Json::Value item;
        item["id"] = row["id"].as<int>();
        item["name"] = row["name"].as<string>();
        item["category"] = row["category"].as<string>();
        item["gender"] = row["gender"].as<string>();
        item["service"] = row["service"].as<string>();
        item["date"] = row["date"].as<string>();
        arr.append(item);
    }
    auto resp = HttpResponse::newHttpJsonResponse(arr);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::reportEarnings(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    string date = req->getParameter("date");
    string masterIdStr = req->getParameter("masterId");
    if (date.empty() || masterIdStr.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing date or masterId";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    int masterId = stoi(masterIdStr);
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT m.name, SUM(s.price - s.discount) as amount "
        "FROM bookings b JOIN services s ON b.service_id = s.id JOIN masters m ON b.master_id = m.id "
        "WHERE b.master_id = $1 AND DATE(b.booking_date) = $2 AND b.status = 'confirmed' "
        "GROUP BY m.name", masterId, date);
    Json::Value respJson;
    if (!result.empty()) {
        respJson["masterName"] = result[0]["name"].as<string>();
        respJson["amount"] = result[0]["amount"].as<int>();
    } else {
        respJson["masterName"] = "";
        respJson["amount"] = 0;
    }
    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::reportPopularService(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT s.name, COUNT(*) as cnt FROM bookings b JOIN services s ON b.service_id = s.id "
        "WHERE b.status = 'confirmed' GROUP BY s.name ORDER BY cnt DESC LIMIT 1");
    Json::Value respJson;
    if (!result.empty()) {
        respJson["name"] = result[0]["name"].as<string>();
        respJson["count"] = result[0]["cnt"].as<int>();
    } else {
        respJson["name"] = "";
        respJson["count"] = 0;
    }
    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::reportGenderRatio(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto male = co_await db->execSqlCoro("SELECT COUNT(*) as cnt FROM clients WHERE gender = 'male'");
    auto female = co_await db->execSqlCoro("SELECT COUNT(*) as cnt FROM clients WHERE gender = 'female'");
    Json::Value respJson;
    respJson["male"] = male[0]["cnt"].as<int>();
    respJson["female"] = female[0]["cnt"].as<int>();
    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::reportPermanentClients(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    string date = req->getParameter("date");
    if (date.empty()) {
        Json::Value err; err["status"]="bad"; err["message"]="Missing date";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k400BadRequest);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT COUNT(DISTINCT u.id) as cnt FROM users u JOIN bookings b ON u.id = b.user_id "
        "WHERE u.category = 'permanent' AND DATE(b.booking_date) = $1 AND b.status = 'confirmed'", date);
    Json::Value respJson;
    respJson["count"] = result[0]["cnt"].as<int>();
    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(k200OK);
    co_return resp;
}

Task<HttpResponsePtr> BusinessController::reportBusiestMaster(const HttpRequestPtr req) {
    auto [isAdmin, userId] = co_await checkAdmin(req);
    if (!isAdmin) {
        Json::Value err; err["status"]="bad"; err["message"]="Forbidden";
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k403Forbidden);
        co_return resp;
    }
    auto db = app().getDbClient("default");
    auto result = co_await db->execSqlCoro(
        "SELECT m.name, COUNT(b.id) as total FROM masters m LEFT JOIN bookings b ON m.id = b.master_id AND b.status = 'confirmed' "
        "GROUP BY m.name ORDER BY total DESC LIMIT 1");
    Json::Value respJson;
    if (!result.empty()) {
        respJson["name"] = result[0]["name"].as<string>();
        respJson["total"] = result[0]["total"].as<int>();
    } else {
        respJson["name"] = "";
        respJson["total"] = 0;
    }
    auto resp = HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(k200OK);
    co_return resp;
}