#pragma once
#include <chrono>
#include <thread>

class Traffic_limiter
{
        public:
                Traffic_limiter(uint64_t bytes_per_sec); // конструктор

                ~Traffic_limiter(); // деструктор

                std::size_t acquire(std::size_t want); // возвращает сколько байт можно переслать и уменьшает счетчик

                void refill(); // обновляет счетчик байт
                
        private:
                std::size_t max_tokens_; // максимальное кол-во байт

                std::size_t tokens_; // сколько байт может переслать на данный момент

                double rate_bytes_per_sec_; // текущая скорость с которой клиент может пересылать данные (байты в секунду)

                std::chrono::steady_clock::time_point last_update_; // когда последний раз клиент пересылал данные

                std::mutex mutex_; // мьютекс для потокобезопасности
};