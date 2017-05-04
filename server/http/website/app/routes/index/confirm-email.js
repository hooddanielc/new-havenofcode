import Ember from 'ember';
import UnauthenticatedRouteMixin from 'ember-simple-auth/mixins/unauthenticated-route-mixin';

export default Ember.Route.extend(UnauthenticatedRouteMixin, {
  model: function (params) {
    return this.store.query('user', {
      email: params.email
    }).then((res) => {
      const user = res.objectAt(0);
      return { error: 'user already active', user };
    }).catch(() => {
      return {
        params: params
      };
    });
  }
});
