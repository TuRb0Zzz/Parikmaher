#pragma once
#include <drogon/drogon.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;

class BusinessController {
public:
    // Публичные
    static Task<HttpResponsePtr> getServices(const HttpRequestPtr req);
    static Task<HttpResponsePtr> getMasters(const HttpRequestPtr req);
    
    // Пользовательские
    static Task<HttpResponsePtr> createBooking(const HttpRequestPtr req);
    static Task<HttpResponsePtr> getUserBookings(const HttpRequestPtr req);
    static Task<HttpResponsePtr> cancelBooking(const HttpRequestPtr req, int bookingId);
    static Task<HttpResponsePtr> rescheduleBooking(const HttpRequestPtr req, int bookingId);
    
    // Админ: клиенты
    static Task<HttpResponsePtr> adminGetClients(const HttpRequestPtr req);
    static Task<HttpResponsePtr> adminCreateClient(const HttpRequestPtr req);
    static Task<HttpResponsePtr> adminDeleteClient(const HttpRequestPtr req, int id);
    static Task<HttpResponsePtr> adminUpdateClient(const HttpRequestPtr req, int id);
    
    // Админ: мастера (с фото)
    static Task<HttpResponsePtr> adminGetMasters(const HttpRequestPtr req);
    static Task<HttpResponsePtr> adminCreateMaster(const HttpRequestPtr req);
    static Task<HttpResponsePtr> adminDeleteMaster(const HttpRequestPtr req, int id);
    
    // Админ: услуги (с фото)
    static Task<HttpResponsePtr> adminGetServices(const HttpRequestPtr req);
    static Task<HttpResponsePtr> adminCreateService(const HttpRequestPtr req);
    static Task<HttpResponsePtr> adminDeleteService(const HttpRequestPtr req, int id);
    
    // Отчёты
    static Task<HttpResponsePtr> reportClientsByDate(const HttpRequestPtr req);
    static Task<HttpResponsePtr> reportEarnings(const HttpRequestPtr req);
    static Task<HttpResponsePtr> reportPopularService(const HttpRequestPtr req);
    static Task<HttpResponsePtr> reportGenderRatio(const HttpRequestPtr req);
    static Task<HttpResponsePtr> reportPermanentClients(const HttpRequestPtr req);
    static Task<HttpResponsePtr> reportBusiestMaster(const HttpRequestPtr req);
};