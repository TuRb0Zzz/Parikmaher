# Парикмахерская
## Стек технологий

- C++20
- Drogon
- PostgreSQL 15
- Docker, Docker Compose
- libpq, libjsoncpp, libuuid, OpenSSL
- React
- React Router DOM
- Context API
- Fetch API

## Требования

- Операционная система: Windows, Linux, macOS
- Установленные Docker и Docker Compose
- Git 
- Node.js 16+
- npm

## Установка и запуск
### 1. Клонирование репозитория

```bash
git clone https://github.com/TuRb0Zzz/Parikmaher.git
cd Parikmaher
```
### 2. Запуск приложения
#### 1. Запуск бэкенда
```bash
docker-compose up --build --d
```
#### 2. Запуск фротенда
Переход в папку и установка зависимостей
```bash
cd frontend
npm install
```
Запуск
```bash
npm start
```

## Завершение работы
### 1. Завершение фронтенда
С сохранением данных
```bash
docker compose down
```
Без сохранения данных
```bash
docker compose down -v
```
### 2. Завершение фронтенда
Достаточно просто нажать Ctrl + C в консоли, откуда он был запущен.