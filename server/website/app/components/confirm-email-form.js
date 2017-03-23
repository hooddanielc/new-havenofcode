import Ember from 'ember';

export default Ember.Component.extend({
  model: null,
  email: null,
  hash: null,
  password: null,
  shakeAnimation: false,
  failed: false,
  loading: false,
  session: Ember.inject.service('session'),

  fail: function () {
    this.set('failed', true);
    this.set('shakeAnimation', true);

    setTimeout(() => {
      this.set('shakeAnimation', false);
    }, 900);
  },

  setParams: function () {
    this.set('email', this.get('model.email'));
    this.set('hash', this.get('model.secret'));
  }.on('init'),

  actions: {
    confirm: function () {
      this.set('loading', true);

      Ember.$.ajax({
        url: '/api/confirm-registration',
        dataType: 'json',
        accept: 'application/json',
        method: 'post',
        data: JSON.stringify({
          user: {
            email: this.get('email'),
            password: this.get('hash'),
            confirmPassword: this.get('password')
          }
        })
      }).then(() => {
        // then login
        return this.get('session').authenticate('authenticator:application', {
          email: this.get('email'),
          password: this.get('password')
        });
      }).then(() => {
        this.set('loading', false);
        this.set('success', true);
      }).catch(() => {
        this.set('loading', false);
        this.fail();
      });
    }
  }
});
