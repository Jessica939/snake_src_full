#ifndef FOOD_TYPE_H
#define FOOD_TYPE_H

enum class FoodType {
    Normal = 0,    // 普通食物 +1
    Special1 = 1,  // 特殊食物1 +2
    Special2 = 2,  // 特殊食物2 +3
    Special3 = 3,  // 特殊食物3 +5
    Poison = 4     // 毒药 -1
};

#endif // FOOD_TYPE_H 
