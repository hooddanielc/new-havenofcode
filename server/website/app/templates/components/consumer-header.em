.hoc-nav
  = link-to 'index.index' class='brand-home'
    img src='/assets/img/logo.png'
    | Haven of Code
  .right-nav
    if session.isAuthenticated
      = link-to 'index.experience-dashboard'
        button.btn My Experiences
      = link-to 'index.logout'
        button.btn Logout
    else
      = link-to 'index.register'
        button.btn Join
      = link-to 'index.login'
        button.btn Login
