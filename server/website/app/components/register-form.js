import Ember from 'ember';

function validateEmail(email) {
  var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
  return re.test(email);
}

function validatePassword(password) {
  return password.length >= 2;
}

export default Ember.Component.extend({
  email: '',
  password: '',
  passwordConfirm: '',
  showIndicatorDelay: 500,

  emailChanged: false,
  passwordChanged: false,
  passwordConfirmChange: false,

  emailValid: false,
  passwordValid: false,
  passwordConfirmValid: false,

  _emailDelay: null,
  _passwordDelay: null,
  _passwordConfirmDelay: null,

  shakeAnimation: false,
  loading: false,
  showConfirmation: false,

  listenEmailChange: function () {
    this.set('emailChanged', false);
    clearTimeout(this._emailDelay);

    this._emailDelay = setTimeout(() => {
      if (this.get('email')) {
        this.set('emailChanged', true);
      }
    }, this.get('showIndicatorDelay'));

    if (validateEmail(this.get('email'))) {
      this.set('emailValid', true);
    } else {
      this.set('emailValid', false);
    }
  }.observes('email'),

  username: function () {
    return this.get('email').split('@')[0];
  }.property('email'),

  checkPassword: function () {
    this.set('passwordChanged', false);
    clearTimeout(this._passwordDelay);

    this._passwordDelay = setTimeout(() => {
      if (this.get('password')) {
        this.set('passwordChanged', true);
      }
    }, this.get('showIndicatorDelay'));

    if (validatePassword(this.get('password'))) {
      this.set('passwordValid', true);
    } else {
      this.set('passwordValid', false);
    }

    this.set('passwordConfirmChanged');
    clearTimeout(this._passwordConfirmDelay);

    this._passwordConfirmDelay = setTimeout(() => {
      if (this.get('passwordConfirm')) {
        this.set('passwordConfirmChanged', true);
      }
    }, this.get('showIndicatorDelay'));

    if (this.get('password') === this.get('passwordConfirm')) {
      this.set('passwordConfirmValid', true);
    } else {
      this.set('passwordConfirmValid', false);
    }
  }.observes('password', 'passwordConfirm'),

  isValid: function () {
    return this.get('passwordValid') === true &&
      this.get('passwordConfirmValid') === true &&
      this.get('emailValid') === true;
  }.property(
    'passwordvalid',
    'passwordConfirmValid',
    'emailValid'
  ),

  shakeButton: function () {
    this.set('shakeAnimation', true);

    setTimeout(() => {
      this.set('shakeAnimation', false);
    }, 900);
  },

  emailDomain: function () {
    return this.get('email').split('@')[1];
  }.property('email'),

  actions: {
    create: function () {
      if (!this.get('isValid')) {
        this.shakeButton();
        return;
      }

      this.set('loading', true);

      Ember.$.ajax({
        url: '/api/register',
        dataType: 'json',
        accept: 'application/json',
        method: 'post',
        data: JSON.stringify({
          user: {
            email: this.get('email'),
            password: this.get('password')
          }
        })
      }).then(() => {
        this.set('loading', false);
        this.set('showConfirmation', true);
      }).catch(() => {
        this.shakeButton();
        this.set('loading', false);
      });
    },

    gotoEmail: function () {
      this.set('loading', true);
      window.location = 'http://' + this.get('emailDomain');
    }
  }
});
