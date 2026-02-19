- Class shouldn't have any public member variables
- All access to member variables needs to be done via functions:
  - Those can be getters/setters
  - Those can forward function calls to member variables - `void A::Refresh() { drawer_->Refresh(); }`

Other patterns are possible. For example, if class A creates class B, it may pass some private variable to class B constructor. `new B(drawer_)` 

Generally, we want a very narrow API for accessing member variables. For example:
 - if we only need to read a variable, then we shouldn't have a setter
 - if a variable is only ever set to true, we don't need a general setter that can set it to true and false `void A::EnableFeature() {feature_ = true;}`
 - if a variable (pointer) is only accessed to call one member function, instead of a getter, we can just have API to only access that function. 
   - `void A::Refresh() { drawer_->Refresh(); }` instead of `A::GetDrawer() { return drawer_; }`