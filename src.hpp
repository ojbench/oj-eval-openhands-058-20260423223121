#ifndef POKEDEX_HPP
#define POKEDEX_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <set>

// Type relationship table
class TypeSystem {
public:
    static float getDamageMultiplier(const std::string& attackType, const std::string& defenseType) {
        // Default is 1.0x
        if (attackType == defenseType) return 1.0f;
        
        // 2x relationships
        if (attackType == "water" && defenseType == "fire") return 2.0f;
        if (attackType == "water" && defenseType == "ground") return 2.0f;
        if (attackType == "fire" && defenseType == "grass") return 2.0f;
        if (attackType == "grass" && defenseType == "water") return 2.0f;
        if (attackType == "grass" && defenseType == "ground") return 2.0f;
        if (attackType == "electric" && defenseType == "water") return 2.0f;
        if (attackType == "electric" && defenseType == "flying") return 2.0f;
        if (attackType == "ground" && defenseType == "electric") return 2.0f;
        if (attackType == "ground" && defenseType == "fire") return 2.0f;
        if (attackType == "flying" && defenseType == "grass") return 2.0f;
        if (attackType == "dragon" && defenseType == "dragon") return 2.0f;
        
        // 0.5x relationships (reverse of 2x)
        if (attackType == "fire" && defenseType == "water") return 0.5f;
        if (attackType == "ground" && defenseType == "water") return 0.5f;
        if (attackType == "grass" && defenseType == "fire") return 0.5f;
        if (attackType == "water" && defenseType == "grass") return 0.5f;
        if (attackType == "ground" && defenseType == "grass") return 0.5f;
        if (attackType == "water" && defenseType == "electric") return 0.5f;
        if (attackType == "flying" && defenseType == "electric") return 0.5f;
        if (attackType == "electric" && defenseType == "ground") return 0.5f;
        if (attackType == "fire" && defenseType == "ground") return 0.5f;
        if (attackType == "grass" && defenseType == "flying") return 0.5f;
        
        // 0x relationships
        if (attackType == "ground" && defenseType == "flying") return 0.0f;
        if (attackType == "electric" && defenseType == "ground") return 0.0f;
        
        return 1.0f;
    }
    
    static bool isValidType(const std::string& type) {
        return type == "water" || type == "fire" || type == "grass" || 
               type == "electric" || type == "ground" || type == "flying" || type == "dragon";
    }
    
    static bool isValidName(const std::string& name) {
        if (name.empty()) return false;
        for (char c : name) {
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                return false;
            }
        }
        return true;
    }
};

class BasicException {
protected:
    std::string message;

public:
    explicit BasicException(const char *_message) : message(_message) {}
    
    virtual const char *what() const {
        return message.c_str();
    }
};

class ArgumentException : public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException : public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector<std::string> types;
    
    Pokemon() : id(0) {
        memset(name, 0, sizeof(name));
    }
    
    Pokemon(const char* _name, int _id, const std::vector<std::string>& _types) : id(_id), types(_types) {
        strncpy(name, _name, 11);
        name[11] = '\0';
    }
    
    std::string getTypesString() const {
        std::string result;
        for (size_t i = 0; i < types.size(); i++) {
            if (i > 0) result += "#";
            result += types[i];
        }
        return result;
    }
};

class Pokedex {
private:
    std::map<int, Pokemon> pokemons;
    std::string fileName;

    std::vector<std::string> splitTypes(const char* types) const {
        std::vector<std::string> result;
        std::string s(types);
        std::stringstream ss(s);
        std::string token;
        while (std::getline(ss, token, '#')) {
            result.push_back(token);
        }
        return result;
    }
    
    void loadFromFile() {
        std::ifstream file(fileName);
        if (!file.is_open()) return;
        
        int count;
        file >> count;
        file.ignore();
        
        for (int i = 0; i < count; i++) {
            Pokemon p;
            std::string nameStr, typesStr;
            file >> nameStr >> p.id;
            file.ignore();
            std::getline(file, typesStr);
            
            strncpy(p.name, nameStr.c_str(), 11);
            p.name[11] = '\0';
            p.types = splitTypes(typesStr.c_str());
            
            pokemons[p.id] = p;
        }
        file.close();
    }
    
    void saveToFile() const {
        std::ofstream file(fileName);
        if (!file.is_open()) return;
        
        file << pokemons.size() << "\n";
        for (const auto& pair : pokemons) {
            const Pokemon& p = pair.second;
            file << p.name << " " << p.id << " " << p.getTypesString() << "\n";
        }
        file.close();
    }

public:
    explicit Pokedex(const char *_fileName) : fileName(_fileName) {
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        // Check for argument exceptions first
        if (!TypeSystem::isValidName(name)) {
            std::string msg = "Argument Error: PM Name Invalid (";
            msg += name;
            msg += ")";
            throw ArgumentException(msg.c_str());
        }
        
        std::vector<std::string> typeVec = splitTypes(types);
        for (const auto& type : typeVec) {
            if (!TypeSystem::isValidType(type)) {
                std::string msg = "Argument Error: PM Type Invalid (";
                msg += type;
                msg += ")";
                throw ArgumentException(msg.c_str());
            }
        }
        
        // Check if id already exists
        if (pokemons.find(id) != pokemons.end()) {
            return false;
        }
        
        Pokemon p(name, id, typeVec);
        pokemons[id] = p;
        return true;
    }

    bool pokeDel(int id) {
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return false;
        }
        pokemons.erase(it);
        return true;
    }

    std::string pokeFind(int id) const {
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return "None";
        }
        return std::string(it->second.name);
    }

    std::string typeFind(const char *types) const {
        // Check for argument exceptions first
        std::vector<std::string> searchTypes = splitTypes(types);
        for (const auto& type : searchTypes) {
            if (!TypeSystem::isValidType(type)) {
                std::string msg = "Argument Error: PM Type Invalid (";
                msg += type;
                msg += ")";
                throw ArgumentException(msg.c_str());
            }
        }
        
        std::set<std::string> searchSet(searchTypes.begin(), searchTypes.end());
        std::vector<std::pair<int, std::string>> results;
        
        for (const auto& pair : pokemons) {
            const Pokemon& p = pair.second;
            bool hasAll = true;
            for (const auto& searchType : searchSet) {
                bool found = false;
                for (const auto& pType : p.types) {
                    if (pType == searchType) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    hasAll = false;
                    break;
                }
            }
            if (hasAll) {
                results.push_back({p.id, std::string(p.name)});
            }
        }
        
        if (results.empty()) {
            return "None";
        }
        
        std::sort(results.begin(), results.end());
        
        std::stringstream ss;
        ss << results.size() << "\n";
        for (const auto& r : results) {
            ss << r.second << "\n";
        }
        std::string result = ss.str();
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        return result;
    }

    float attack(const char *type, int id) const {
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return -1.0f;
        }
        
        const Pokemon& defender = it->second;
        float totalMultiplier = 1.0f;
        
        for (const auto& defType : defender.types) {
            totalMultiplier *= TypeSystem::getDamageMultiplier(type, defType);
        }
        
        return totalMultiplier;
    }

    int catchTry() const {
        if (pokemons.empty()) return 0;
        
        std::set<int> owned;
        std::set<int> canCatch;
        
        // Start with the smallest id
        owned.insert(pokemons.begin()->first);
        
        bool changed = true;
        while (changed) {
            changed = false;
            canCatch.clear();
            
            // Try to catch each pokemon not owned
            for (const auto& targetPair : pokemons) {
                int targetId = targetPair.first;
                if (owned.find(targetId) != owned.end()) continue;
                
                const Pokemon& target = targetPair.second;
                
                // Check if any owned pokemon can catch it
                bool canBeCaught = false;
                for (int ownedId : owned) {
                    const Pokemon& attacker = pokemons.at(ownedId);
                    
                    // Try each type of the attacker
                    for (const auto& attackType : attacker.types) {
                        float damage = 1.0f;
                        for (const auto& defType : target.types) {
                            damage *= TypeSystem::getDamageMultiplier(attackType, defType);
                        }
                        
                        if (damage >= 2.0f) {
                            canBeCaught = true;
                            break;
                        }
                    }
                    if (canBeCaught) break;
                }
                
                if (canBeCaught) {
                    canCatch.insert(targetId);
                }
            }
            
            // Add all catchable pokemon to owned
            if (!canCatch.empty()) {
                for (int id : canCatch) {
                    owned.insert(id);
                }
                changed = true;
            }
        }
        
        return owned.size();
    }

    struct iterator {
        std::map<int, Pokemon>::iterator it;
        std::map<int, Pokemon>* container;
        
        iterator() : container(nullptr) {}
        iterator(std::map<int, Pokemon>::iterator _it, std::map<int, Pokemon>* _container) 
            : it(_it), container(_container) {}

        iterator &operator++() {
            if (container == nullptr || it == container->end()) {
                throw IteratorException("Iterator Error: Iterator Out of Bound");
            }
            ++it;
            return *this;
        }
        
        iterator &operator--() {
            if (container == nullptr || it == container->begin()) {
                throw IteratorException("Iterator Error: Iterator Out of Bound");
            }
            --it;
            return *this;
        }
        
        iterator operator++(int) {
            if (container == nullptr || it == container->end()) {
                throw IteratorException("Iterator Error: Iterator Out of Bound");
            }
            iterator temp = *this;
            ++it;
            return temp;
        }
        
        iterator operator--(int) {
            if (container == nullptr || it == container->begin()) {
                throw IteratorException("Iterator Error: Iterator Out of Bound");
            }
            iterator temp = *this;
            --it;
            return temp;
        }
        
        iterator & operator = (const iterator &rhs) {
            it = rhs.it;
            container = rhs.container;
            return *this;
        }
        
        bool operator == (const iterator &rhs) const {
            return it == rhs.it;
        }
        
        bool operator != (const iterator &rhs) const {
            return it != rhs.it;
        }
        
        Pokemon & operator*() const {
            if (container == nullptr || it == container->end()) {
                throw IteratorException("Iterator Error: Invalid Dereference");
            }
            return it->second;
        }
        
        Pokemon *operator->() const {
            if (container == nullptr || it == container->end()) {
                throw IteratorException("Iterator Error: Invalid Dereference");
            }
            return &(it->second);
        }
    };

    iterator begin() {
        return iterator(pokemons.begin(), &pokemons);
    }

    iterator end() {
        return iterator(pokemons.end(), &pokemons);
    }
};

#endif
