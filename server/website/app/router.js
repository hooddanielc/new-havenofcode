import Ember from 'ember';
import config from './config/environment';

const Router = Ember.Router.extend({
  location: config.locationType,
  rootURL: config.rootURL
});

Router.map(function() {
  this.route('index', { path: '/' }, function () {
    this.route('index', { path: '/' });
    this.route('register');
    this.route('confirm-email', { path: '/confirm-email/:secret/:email'});
    this.route('login');
    this.route('logout');
    this.route('experience-dashboard');
  });
});

export default Router;
