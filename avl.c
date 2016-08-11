#include "ruby.h"
#include <stdio.h>

static VALUE cAVL, cAVLNode;
static ID id_key, id_val, id_left, id_right, id_height, id_root, id_comp;

#define get_key(x) (rb_ivar_get((x), id_key))
#define get_left(x) (rb_ivar_get((x), id_left))
#define get_right(x) (rb_ivar_get((x), id_right))

VALUE
avlnode_inilialize(VALUE self, VALUE key, VALUE val){
    rb_ivar_set(self, id_key, key);
    rb_ivar_set(self, id_val, val);
    rb_ivar_set(self, id_left, Qnil);
    rb_ivar_set(self, id_right, Qnil);
    rb_ivar_set(self, id_height, LONG2FIX(1));
    return self;
}

long get_long_height(VALUE node) {
    if (NIL_P(node)) {
        return 0;
    }
    else {
        return FIX2LONG(rb_ivar_get(node, id_height));
    }
}

VALUE
avlnode_retrieve(VALUE self, VALUE key){
    long result_comp;

    if (NIL_P(self)) {
        return Qnil;
    }

    result_comp = FIX2LONG(rb_funcall(key, id_comp, 1, get_key(self)));
    switch (result_comp) {
    case -1: // key is smaller than self.key
        return avlnode_retrieve(get_left(self), key);
    case  0: // key equals self.key
        return rb_ivar_get(self, id_val);
    case  1: // key is lager than self.key
        return avlnode_retrieve(get_right(self), key);
    default:
        rb_raise(rb_eTypeError, "key can not compare by <=>");
    }
}

void
avlnode_update_height(VALUE node){
    long left_height, right_height, new_height;

    if (NIL_P(node)) {
        return;
    }

    left_height = get_long_height(get_left(node));
    right_height = get_long_height(get_right(node));
    new_height = (left_height > right_height ? left_height : right_height) + 1;
    rb_ivar_set(node, id_height, LONG2FIX(new_height));
}

VALUE
avlnode_rotate_left(VALUE self){
    VALUE root;

    if (NIL_P(self)) {
        return Qnil;
    }

    root = rb_ivar_get(self, id_right);
    rb_ivar_set(self, id_right, rb_ivar_get(root, id_left));
    rb_ivar_set(root, id_left, self);
    avlnode_update_height(self);
    return root;
}

VALUE
avlnode_rotate_right(VALUE self){
    VALUE root;

    if (NIL_P(self)) {
        return Qnil;
    }

    root = rb_ivar_get(self, id_left);
    rb_ivar_set(self, id_left, rb_ivar_get(root, id_right));
    rb_ivar_set(root, id_right, self);
    avlnode_update_height(self);
    return root;
}

VALUE
avlnode_rotate(VALUE self){
    long left_height, right_height;
    VALUE left, right, root;

    if (NIL_P(self)) {
        return Qnil;
    }

    left = get_left(self);
    right = get_right(self);
    left_height  = get_long_height(left);
    right_height = get_long_height(right);

    switch (left_height - right_height) {
    case 2:
        left_height  = get_long_height(get_left(left));
        right_height = get_long_height(get_right(left));
        if (left_height < right_height) {
            rb_ivar_set(self, id_left, avlnode_rotate_left(left));
        }
        root = avlnode_rotate_right(self);
        break;
    case -2:
        left_height  = get_long_height(get_left(right));
        right_height = get_long_height(get_right(right));
        if (left_height > right_height) {
            rb_ivar_set(self, id_right, avlnode_rotate_right(right));
        }
        root = avlnode_rotate_left(self);
        break;
    default:
        root = self;
    }
    avlnode_update_height(root);
    return root;
}

VALUE
avlnode_insert(VALUE self, VALUE key , VALUE val){
    VALUE child;
    long result_comp;

    if (NIL_P(self)) {
        self = rb_obj_alloc(cAVLNode);
        self = avlnode_inilialize(self, key, val);
        return self;
    }

    result_comp = FIX2LONG(rb_funcall(key, id_comp, 1, get_key(self)));
    switch (result_comp) {
    case -1: // key is smaller than self.key
        child = avlnode_insert(rb_ivar_get(self, id_left), key, val);
        rb_ivar_set(self, id_left, child);
        break;
    case  0: // key equals self.key
        rb_ivar_set(self, id_val, val);
        break;
    case  1: // key is lager than self.key
        child = avlnode_insert(rb_ivar_get(self, id_right), key, val);
        rb_ivar_set(self, id_right, child);
        break;
    default:
        rb_raise(rb_eTypeError, "key can not compare by <=>");
    }

    return avlnode_rotate(self);
}

void
avlnode_each(VALUE self){
    VALUE key, val;

    if (NIL_P(self)) {
        return;
    }

    avlnode_each(get_left(self));

    key = rb_ivar_get(self, id_key);
    val = rb_ivar_get(self, id_val);
    rb_yield(rb_assoc_new(key, val));

    avlnode_each(get_right(self));
}

VALUE
avl_inilialize(VALUE self){
    rb_ivar_set(self, id_root, Qnil);
    return self;
}

VALUE
avl_retrieve(VALUE self, VALUE key){
    VALUE root;

    root = rb_ivar_get(self, id_root);
    return avlnode_retrieve(root, key);
}

VALUE
avl_insert(VALUE self, VALUE key, VALUE val){
    VALUE root;

    root = rb_ivar_get(self, id_root);
    root = avlnode_insert(root, key, val);
    rb_ivar_set(self, id_root, root);
    return val;
}

VALUE
avl_each(VALUE self){
    VALUE root;

    root = rb_ivar_get(self, id_root);
    avlnode_each(root);
    return self;
}

void
Init_avl(void) {
    cAVL = rb_define_class("AVL", rb_cObject);
    rb_define_method(cAVL, "initialize", avl_inilialize, 0);
    rb_define_method(cAVL, "retrieve", avl_retrieve, 1);
    rb_define_method(cAVL, "[]", avl_retrieve, 1);
    rb_define_method(cAVL, "insert", avl_insert, 2);
    rb_define_method(cAVL, "[]=", avl_insert, 2);
    rb_define_method(cAVL, "each", avl_each, 0);

    cAVLNode = rb_define_class("AVLNode", rb_cObject);
    rb_define_method(cAVLNode, "initialize", avlnode_inilialize, 2);

    id_key = rb_intern("@key");
    id_val = rb_intern("@val");
    id_left = rb_intern("@left");
    id_right = rb_intern("@right");
    id_height = rb_intern("@height");
    id_root = rb_intern("@root");
    id_comp = rb_intern("<=>");
}
