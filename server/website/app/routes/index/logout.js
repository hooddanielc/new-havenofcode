import Ember from 'ember';
import AuthenticatedRouteMixin from 'ember-simple-auth/mixins/authenticated-route-mixin';

export default Ember.Route.extend(AuthenticatedRouteMixin, {
  session: Ember.inject.service('session'),

  beforeModel: function () {
    this.get('session').invalidate().then(() => {
      this.transitionTo('index.login');
    });
  }
});
