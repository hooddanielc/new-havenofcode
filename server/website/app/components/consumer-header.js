import Ember from 'ember';

export default Ember.Component.extend({
  session: Ember.inject.service('session'),

  loadUser: function () {
    if (this.get('session.isAuthenticated')) {
      console.log('authenticated');
    }
  }.on('didInsertElement')
});
