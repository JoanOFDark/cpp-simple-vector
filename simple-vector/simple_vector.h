#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iterator>

using namespace std::literals;

struct ReserveProxyObj {
    ReserveProxyObj(size_t capacity_to_reserve) : capacity_(capacity_to_reserve) {}

    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveProxyObj rpo) {
        this->Reserve(rpo.capacity_);
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size) {
        for (auto it = this->begin(); it != this->end(); ++it) {
            *it = std::move(Type{});
        }
        //std::fill(this->begin(), this->end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(this->begin(), this->end(), value);
    }

    SimpleVector(size_t size, Type&& value) : SimpleVector(size) {
        for (auto it = this->begin(); it != this->end(); ++it) {
            *it = std::move(value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
        /*if (std::empty(init)) {
            return;
        }
        size_t i = 0;
        for (auto it = std::cbegin(init); it != std::cend(init); ++it) {
            items_[i++] = *it;
        }*/
    }

    // Конструктор с копированием
    SimpleVector(const SimpleVector& other)  { 
        SimpleVector temp(other.GetSize());
        std::copy(other.begin(), other.end(), &temp[0]);
        (*this).swap(temp);
    }

    // Конструктор с перемещением
    SimpleVector(SimpleVector&& other) : items_(std::move(other.items_)) { //чуть не забыл написать конструктор перемещения в array_ptr))
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        // Напишите тело самостоятельно
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert((index < size_), true);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert((index < size_), true);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size"s);
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        this->Resize(0);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size > size_ && new_size <= capacity_) {
            //std::fill(items_.Get() + size_, items_.Get() + new_size, Type{}); 
            for (auto it = this->begin() + size_; it != this->begin() + new_size; ++it) {
                *it = std::move(Type());
            }
            size_ = new_size;
        }
        else if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_items(new_capacity);

            std::move(items_.Get(), items_.Get() + size_, new_items.Get());
            //std::fill(new_items.Get() + size_, new_items.Get() + new_size, Type{});
            for (auto it = new_items.Get() + size_; it != new_items.Get() + new_size; ++it) {
                *it = std::move(Type());
            }
            items_.swap(new_items);

            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator(items_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator(items_.Get() + size_);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(items_.Get());
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator(items_.Get() + size_);
    }

    // Присваивание с копированием
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this == rhs) {
            return *this;
        }
        SimpleVector temp(rhs);
        this->swap(temp);
        return *this;
    }

    // Присваивание с перемещением
    SimpleVector& operator=(SimpleVector&& rhs) {
        if (items_.Get() != rhs.items_.Get()) {
            ArrayPtr<Type> temp(rhs.size_);
            std::move(std::make_move_iterator(rhs.items_.Get()), std::make_move_iterator(rhs.items_.Get() + rhs.size_), temp.Get());
            items_.swap(temp);
            size_ = std::move(rhs.size_);
            capacity_ = std::move(rhs.size_);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    // Добавление с копированием
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            SimpleVector temp_svec(new_capacity);
            std::copy(this->begin(), this->end(), &temp_svec[0]);
            items_.swap(temp_svec.items_);
            capacity_ = new_capacity;
        }
        items_[size_++] = item;
    }

    // Добавление с перемещением
    void PushBack(Type&& item)
    {
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : size_ * 2);
            SimpleVector temp_svec(new_capacity);
            std::move(this->begin(), this->end(), &temp_svec[0]);
            items_.swap(temp_svec.items_);
            capacity_ = new_capacity;
        }
        items_[size_++] = std::exchange(item, {});
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert((pos >= begin() && pos <= end()), true);
        if (size_ == capacity_) {
            size_t temp_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            SimpleVector temp_svec(temp_capacity);

            std::copy(this->cbegin(), pos, &temp_svec[0]);
            Iterator result = std::copy_backward(pos, this->cend() - 1, &temp_svec[++size_]);
            *(--result) = value;
            items_.swap(temp_svec.items_);

            capacity_ = temp_capacity;
            return result;
        }

        Iterator result = std::copy_backward(pos, this->cend() - 1, &items_[++size_]);
        *(--result) = value;
        return result;
    }

    // Вставка с перемещением
    Iterator Insert(Iterator pos, Type&& value) {
        assert((pos >= begin() && pos <= end()), true);
        if (size_ == capacity_) {
            size_t temp_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            SimpleVector temp_svec(temp_capacity);

            std::move(this->begin(), pos, &temp_svec[0]);
            Iterator result = std::move_backward(pos, this->end() - 1, &temp_svec[++size_]);
            *(--result) = std::move(value);
            items_.swap(temp_svec.items_);

            capacity_ = temp_capacity;
            return result;
        }

        Iterator result = std::move_backward(pos, this->end() - 1, &items_[++size_]);
        *(--result) = std::move(value);
        return result;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert((pos >= begin() && pos < end()), true);
        if (IsEmpty()) {
            return this->end();
        }
        Iterator res = const_cast<Iterator>(pos); //не красиво
        std::move(res + 1, this->end(), res);
        --size_;    
        return res;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(other.size_, this->size_);
        std::swap(other.capacity_, this->capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector temp(new_capacity);
            std::copy(this->begin(), this->end(), &temp[0]);
            items_.swap(temp.items_);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return ((lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()))
        || (lhs.begin() == rhs.begin()); //самосравнение
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs) && !(lhs == rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs > rhs || lhs == rhs;
}
