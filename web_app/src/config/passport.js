const passport = require('passport');
const LocalStrategy = require('passport-local').Strategy;

const User = require('../models/User'); 

// Nueva estrategia para login
passport.use(new LocalStrategy({
  usernameField: "email",
  passwordField: "password"
}, async (email, password, done) => {
  // Match Email's User
  const user = await User.findOne({email: email});
  if (!user) {
    return done(null, false, { message: 'Not User found.' });
  } else {
    // Match Password's User
    const match = await user.matchPassword(password);
    if(match) {
      return done(null, user);
    } else {
      return done(null, false, { message: 'Incorrect Password.' });
    }
  }
}));

// Guarda el ID del usuario 
passport.serializeUser((user, done) => {
  done(null, user.id);
});

// a partir de un ID, te devuelve un usuario
passport.deserializeUser((id, done) => {
  User.findById(id, (err, user) => {
    done(err, user);
  });
});