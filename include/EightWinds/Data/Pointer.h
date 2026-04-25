#pragma once

namespace EWE{
    template<typename T>
    struct OptionalReference{
        T* ref;

        operator bool() const{
            return ref != nullptr;
        }

        T& operator->(){
            return *ref;
        }
        T& operator*(){
            return *ref;
        }
    }:
}