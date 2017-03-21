import Ember from 'ember';

export default Ember.Component.extend({
  email: null,
  password: null,
  session: Ember.inject.service('session'),
  shakeAnimation: false,
  ctrl: null,

  shake: function () {
    this.set('shakeAnimation', true);

    setTimeout(() => {
      this.set('shakeAnimation', false);
    }, 900);
  },

  actions: {
    login: function () {
      this.get('session').authenticate('authenticator:application', {
        email: this.get('email'),
        password: this.get('password')
      }).then(() => {
        this.get('ctrl').transitionToRoute('index');
      }).catch(() => {
        this.shake();
      });
    }
  }
});
