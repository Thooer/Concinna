#pragma once
import <string>;
import <vector>;

namespace Foundation::IRSystem {

struct Role {
    std::string name;
};

struct Field {
    std::string name;
};

struct ObjectDescriptor {
    std::vector<Field> fields;
    std::vector<Role> roles;
    bool validate() const;
};

}