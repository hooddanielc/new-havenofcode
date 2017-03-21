.login-form-container
  form{action "login" on="submit"}
    .user-icon
      span.glyphicon.glyphicon-user
    .form-group
      label for="email" Email Address
      = input value=email type="email" class="form-control input-lg" id="email" placeholder="Email"

    .form-group
      label for="password" Password
      = input value=password type="password" class="form-control input-lg" id="password" placeholder="Password"

    button.btn.btn-primary.btn-lg class="{{if shakeAnimation 'incorrect-submit' '' }}" type="submit" Login
