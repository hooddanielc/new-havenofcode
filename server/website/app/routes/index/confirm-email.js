import Ember from 'ember';

export default Ember.Route.extend({
  model: function (params) {
    return this.store.query('user', {
      email: params.email
    }).then((res) => {
      const user = res.objectAt(0);

      if (!user) {
        return { error: 'user missing' };
      }

      if (user.get('active')) {
        return { error: 'user already active' };
      }

      return {
        user: user,
        params: params
      };
    });
  }
});
