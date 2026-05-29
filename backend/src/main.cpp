#include "AuthController.h"
#include "BusinessController.h"
#include <drogon/drogon.h>
#include <cstdlib>
#include <string>

using namespace drogon;

int main() {
    const char* dbHost = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "postgres";
    unsigned short dbPort = std::getenv("DB_PORT") ? static_cast<unsigned short>(std::stoi(std::getenv("DB_PORT"))) : 5432;
    const char* dbName = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "mydb";
    const char* dbUser = std::getenv("DB_USER") ? std::getenv("DB_USER") : "postgres";
    const char* dbPassword = std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD") : "123456";

    app().createDbClient("postgresql", dbHost, dbPort, dbName, dbUser, dbPassword, 4, "", "default");

    app().registerPreRoutingAdvice([](const HttpRequestPtr &req, FilterCallback &&defer, FilterChainCallback &&chain) {
        if (req->method() == Options) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k204NoContent);
            std::string origin = req->getHeader("Origin");
            if (origin.empty()) origin = "http://localhost:3000";
            resp->addHeader("Access-Control-Allow-Origin", origin);
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Accept, Authorization, X-Requested-With, Cookie");
            resp->addHeader("Access-Control-Allow-Credentials", "true");
            resp->addHeader("Access-Control-Max-Age", "3600");
            defer(resp);
            return;
        }
        chain();
    });

    app().registerPostHandlingAdvice([](const HttpRequestPtr &req, const HttpResponsePtr &resp) {
        std::string origin = req->getHeader("Origin");
        if (!origin.empty()) {
            resp->addHeader("Access-Control-Allow-Origin", origin);
            resp->addHeader("Access-Control-Allow-Credentials", "true");
        }
    });

    app().setDocumentRoot("./images");

    app().registerHandler("/api/auth/register", &AuthController::registerUser, {Post});
    app().registerHandler("/api/auth/login", &AuthController::loginUser, {Post});
    app().registerHandler("/api/auth/me", &AuthController::me, {Get});
    app().registerHandler("/api/auth/logout", &AuthController::logout, {Post});

    app().registerHandler("/api/services", &BusinessController::getServices, {Get});
    app().registerHandler("/api/masters", &BusinessController::getMasters, {Get});

    app().registerHandler("/api/bookings", &BusinessController::createBooking, {Post});
    app().registerHandler("/api/bookings/user", &BusinessController::getUserBookings, {Get});
    app().registerHandler("/api/bookings/{1}", &BusinessController::cancelBooking, {Delete});
    app().registerHandler("/api/bookings/{1}/reschedule", &BusinessController::rescheduleBooking, {Put});

    app().registerHandler("/api/admin/clients", &BusinessController::adminGetClients, {Get});
    app().registerHandler("/api/admin/clients", &BusinessController::adminCreateClient, {Post});
    app().registerHandler("/api/admin/clients/{1}", &BusinessController::adminDeleteClient, {Delete});
    app().registerHandler("/api/admin/clients/{1}", &BusinessController::adminUpdateClient, {Put});

    app().registerHandler("/api/admin/masters", &BusinessController::adminGetMasters, {Get});
    app().registerHandler("/api/admin/masters", &BusinessController::adminCreateMaster, {Post});
    app().registerHandler("/api/admin/masters/{1}", &BusinessController::adminDeleteMaster, {Delete});
    app().registerHandler("/api/admin/masters/{1}", &BusinessController::adminUpdateMaster, {Put});

    app().registerHandler("/api/admin/services", &BusinessController::adminGetServices, {Get});
    app().registerHandler("/api/admin/services", &BusinessController::adminCreateService, {Post});
    app().registerHandler("/api/admin/services/{1}", &BusinessController::adminDeleteService, {Delete});
    app().registerHandler("/api/admin/services/{1}", &BusinessController::adminUpdateService, {Put});

    app().registerHandler("/api/admin/bookings", &BusinessController::adminGetAllBookings, {Get});
    app().registerHandler("/api/admin/bookings/{1}/confirm", &BusinessController::adminConfirmBooking, {Put});

    app().registerHandler("/api/admin/reports/clients", &BusinessController::reportClientsByDate, {Get});
    app().registerHandler("/api/admin/reports/earnings", &BusinessController::reportEarnings, {Get});
    app().registerHandler("/api/admin/reports/popular-service", &BusinessController::reportPopularService, {Get});
    app().registerHandler("/api/admin/reports/gender-ratio", &BusinessController::reportGenderRatio, {Get});
    app().registerHandler("/api/admin/reports/permanent-clients", &BusinessController::reportPermanentClients, {Get});
    app().registerHandler("/api/admin/reports/busiest-master", &BusinessController::reportBusiestMaster, {Get});

    app().addListener("0.0.0.0", 8080).run();
    return 0;
}