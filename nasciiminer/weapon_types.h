#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdint.h>

typedef enum {
    WEAPON_TYPE_NONE      = 0,
    WEAPON_TYPE_PISTOL    = 1,
    WEAPON_TYPE_SHOTGUN   = 2,
    WEAPON_TYPE_SMG       = 3,
    WEAPON_TYPE_RIFLE     = 4,
    WEAPON_TYPE_AUTORIFLE = 5,
    WEAPON_TYPE_LAUNCHER  = 6,
} weapon_type_t;

typedef enum {
    WEAPON_ELEMENT_NONE   = 0,
    WEAPON_ELEMENT_FLAME  = 1,
    WEAPON_ELEMENT_FREEZE = 2,
    WEAPON_ELEMENT_SHOCK  = 3,
} weapon_type_t;

typedef struct weapon_component_barrel {
    uint32_t weight;
    uint32_t length;
} weapon_barrel_t;

typedef struct weapon_component_receiver {
    uint32_t weight;
    uint32_t magazine_size;
} weapon_receiver_t;

typedef struct weapon_component_scope {
    uint32_t magnification;
} weapon_scope_t;

typedef struct weapon_component_stock {
    uint32_t weight;
    uint32_t length;
} weapon_stock_t;

typedef struct weapon {
    bool initialized;
    /* generic stats */
    uint32_t base_damage;
    uint32_t accuracy;
    uint32_t fire_rate;
    /* modifiers */
    weapon_type_t type;
    /* components */
    weapon_barrel_t barrel;
    weapon_receiver_t receiver;
    weapon_scope_t scope;
    weapon_stock_t stock;
} weapon_t;


#endif
