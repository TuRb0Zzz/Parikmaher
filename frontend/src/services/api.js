const API_BASE = 'http://localhost:8080/api';

async function request(endpoint, options = {}) {
  const response = await fetch(`${API_BASE}${endpoint}`, {
    ...options,
    credentials: 'include',
    headers: options.body instanceof FormData ? undefined : { 'Content-Type': 'application/json', ...options.headers },
  });
  if (!response.ok) {
    const error = await response.json().catch(() => ({}));
    throw new Error(error.message || 'Request failed');
  }
  return response.json();
}

export const api = {
  auth: {
    login: async (email, password) => {
      const res = await request('/auth/login', { method: 'POST', body: JSON.stringify({ email, password }) });
      return res.user;
    },
    register: async (userData) => {
      const res = await request('/auth/register', { method: 'POST', body: JSON.stringify(userData) });
      return res.user;
    },
    me: async () => {
      const res = await request('/auth/me');
      return res.user;
    },
    logout: () => request('/auth/logout', { method: 'POST' }),
  },
  services: {
    getAll: () => request('/services'),
    book: (serviceId, masterId, date) => request('/bookings', {
      method: 'POST',
      body: JSON.stringify({ serviceId, masterId, date }),
    }),
  },
  masters: {
    getAll: () => request('/masters'),
  },
  bookings: {
    getUserBookings: () => request('/bookings/user'),
    cancel: (bookingId) => request(`/bookings/${bookingId}`, { method: 'DELETE' }),
    reschedule: (bookingId, newDate) => request(`/bookings/${bookingId}/reschedule`, {
      method: 'PUT',
      body: JSON.stringify({ date: newDate }),
    }),
  },
  admin: {
    getClients: () => request('/admin/clients'),
    addClient: (client) => request('/admin/clients', { method: 'POST', body: JSON.stringify(client) }),
    deleteClient: (id) => request(`/admin/clients/${id}`, { method: 'DELETE' }),
    updateClient: (id, data) => request(`/admin/clients/${id}`, { method: 'PUT', body: JSON.stringify(data) }),
    getMasters: () => request('/admin/masters'),
    addMaster: (formData) => request('/admin/masters', { method: 'POST', body: formData }),
    deleteMaster: (id) => request(`/admin/masters/${id}`, { method: 'DELETE' }),
    getServices: () => request('/admin/services'),
    addService: (formData) => request('/admin/services', { method: 'POST', body: formData }),
    deleteService: (id) => request(`/admin/services/${id}`, { method: 'DELETE' }),
    getReports: {
      clientsByDate: (date) => request(`/admin/reports/clients?date=${date}`),
      masterEarnings: (date, masterId) => request(`/admin/reports/earnings?date=${date}&masterId=${masterId}`),
      mostPopularService: () => request('/admin/reports/popular-service'),
      genderRatio: () => request('/admin/reports/gender-ratio'),
      permanentClientsCount: (date) => request(`/admin/reports/permanent-clients?date=${date}`),
      busiestMaster: () => request('/admin/reports/busiest-master'),
    },
  },
};