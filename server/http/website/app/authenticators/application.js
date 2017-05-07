import Ember from 'ember';
import Base from 'ember-simple-auth/authenticators/base';

export default Base.extend({
  store: Ember.inject.service('store'),

  restore: function (data) {
    return this.get('store').findRecord('user', 'me').then((res) => {
      return data;
    });
  },

  authenticate: function (options) {
    return Ember.$.ajax({
      url: '/api/login',
      dataType: 'json',
      accept: 'application/json',
      method: 'post',
      data: JSON.stringify({
        email: options.email,
        password: options.password
      })
    }).then((res) => {
      return res.user;
    });
  },

  invalidate: function () {
    return Ember.$.ajax({
      url: '/api/logout'
    });
  }
});
