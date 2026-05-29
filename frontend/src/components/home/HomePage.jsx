import React from 'react';
import './HomePage.css';

function HomePage() {
  return (
    <div className="homepage">
      <div className="hero-section">
        <div className="hero-animation">
          
          <div className="hero-overlay"></div>
        </div>
        <div className="hero-slide-image">
          <img src="/barber-photo.jpg" alt="Парикмахер" />
        </div>
        <div className="hero-content">
          <h1 className="hero-title">HairCraft</h1>
          <p className="hero-subtitle">Стиль, который вам идёт</p>
        </div>
      </div>

      <div className="about-section">
        <div className="container">
          <h2>О нас</h2>
          <p>HairCraft – это современная парикмахерская, где мастерство встречается с комфортом. Наши мастера – профессионалы своего дела, которые помогут подчеркнуть вашу индивидуальность. Мы используем только качественные материалы и следим за трендами.</p>
          <div className="features">
            <div className="feature">
              <h3>Профессионалы</h3>
              <p>Опытные мастера с высокими разрядами</p>
            </div>
            <div className="feature">
              <h3>Любые причёски</h3>
              <p>Классика, креатив, укладки</p>
            </div>
            <div className="feature">
              <h3>Удобная запись</h3>
              <p>Онлайн-бронирование 24/7</p>
            </div>
          </div>
        </div>
      </div>

      <div className="gallery-section">
        <div className="container">
          <h2>Наши работы</h2>
          <div className="gallery">
            <img src="/work1.jpg" alt="Работа 1" />
            <img src="/work2.jpg" alt="Работа 2" />
            <img src="/work3.jpg" alt="Работа 3" />
          </div>
        </div>
      </div>
    </div>
  );
}

export default HomePage;