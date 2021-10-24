#ifndef DETSVR_SERIALIZE_H
#define DETSVR_SERIALIZE_H

#include "nlohmann/json.hpp"
#include <string>

namespace detsvr
{

template<typename T>
struct ISerialize
{
    static std::string Serialize(const T& obj);
    static void Deserialize(T& obj, const std::string& s);
};

template <typename T>
std::string ISerialize<T>::Serialize(const T& obj)
{
    nlohmann::ordered_json j = obj;
    return j.dump(4);
}

template<typename T>
void ISerialize<T>::Deserialize(T& obj, const std::string& s)
{
    nlohmann::json j = nlohmann::json::parse(s);
    j.get_to(obj);
}

} // end namespace detsvr

#endif