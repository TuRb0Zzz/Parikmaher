import React, { useState, useEffect } from 'react';
import { useAuth } from '../../contexts/AuthContext';
import { api } from '../../services/api';
import BookingModal from './BookingModal';
import './ServicesPage.css';

function ServicesPage() {
  const { user } = useAuth();
  const [services, setServices] = useState([]);
  const [selectedHall, setSelectedHall] = useState('all');
  const [modalService, setModalService] = useState(null);
  const [message, setMessage] = useState('');

  useEffect(() => {
    api.services.getAll()
      .then(data => setServices(Array.isArray(data) ? data : []))
      .catch(err => console.error(err));
  }, []);

  const filteredServices = selectedHall === 'all' ? services : services.filter(s => s.gender === selectedHall);

  const handleCardClick = (service) => {
    if (!user) {
      setMessage('Для записи необходимо войти в систему');
      setTimeout(() => setMessage(''), 3000);
      return;
    }
    setModalService(service);
  };

  const handleBookingSuccess = () => {
    alert('Запись успешно создана!');
  };

  return (
    <div className="services-page">
      <h1 className="page-title">Наши услуги</h1>
      
      <div className="hall-filter">
        <button className={`filter-btn ${selectedHall === 'all' ? 'active' : ''}`} onClick={() => setSelectedHall('all')}>Все</button>
        <button className={`filter-btn ${selectedHall === 'male' ? 'active' : ''}`} onClick={() => setSelectedHall('male')}>Мужские</button>
        <button className={`filter-btn ${selectedHall === 'female' ? 'active' : ''}`} onClick={() => setSelectedHall('female')}>Женские</button>
      </div>

      {message && <div className="warning-message">{message}</div>}

      <div className="services-grid">
        {filteredServices.map(service => (
          <div key={service.id} className="service-card" onClick={() => handleCardClick(service)}>
            <img 
              src={service.photo_url || '/placeholder-service.png'} 
              alt={service.name} 
              className="service-photo" 
            />
            <h3 className="service-name">{service.name}</h3>
            <p className="service-price">{service.price} ₽</p>
            <p className="service-discount">Скидка пост. {service.discount}%</p>
          </div>
        ))}
      </div>

      {modalService && (
        <BookingModal 
          service={modalService} 
          onClose={() => setModalService(null)} 
          onSuccess={handleBookingSuccess}
        />
      )}
    </div>
  );
}

export default ServicesPage;