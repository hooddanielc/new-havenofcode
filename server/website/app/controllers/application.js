import Ember from 'ember';

export default Ember.Controller.extend({
  actions: {
    login: function () {
      console.log('Loggedin?');
    }
  }
})