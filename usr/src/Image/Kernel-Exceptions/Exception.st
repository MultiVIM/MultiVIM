"Block extend [
    ensure: terminationBlock [
        Evaluate the receiver and return its result. Immediately after successful evaluation of the receiver
        but before returning its result, evaluate terminationBlock. If abnormal termination of the 
        receiver occurs, terminationBlock is evaluated. In either case, the value returned from the
        evaluation of terminationBlock is discarded.

        Activation of an exception handler from within the receiver is not in and of itself an abnormal
        termination. However, if the exception handler for an exception that is not resumable results in
        termination of the receiver or if its handler block contains a return statement that results in
        abnormal termination of the receiver, then terminationBlock will be evaluated after evaluation
        of the exception handler.

        If an abnormal termination results in the termination of multiple blocks which were evaluated using
        either #ensure: or #ifCurtailed: the respective terminationBlocks will be executed in
        the reverse of the order in which the corresponding receiver blocks were evaluated. 
    ]
    
    ifCurtailed: terminationBlock [
        Evaluate the receiver and return its result. If abnormal termination of the receiver occurs,
        terminationBlock is evaluated. The value returned from the evaluation of
        terminationBlock is discarded.

        Activation of an exception handler from within the receiver is not in and of itself an abnormal
        termination. However, if the exception handler for an exception that is not resumable results in
        termination of the receiver or if its handler block contains a return statement that results in
        abnormal termination of the receiver, then terminationBlock will be evaluated after evaluation
        of the exception handler.

        If an abnormal termination result in the termination of multiple blocks which were evaluated using
        either #ensure: or #ifCurtailed: the respective terminationBlocks will be executed in
        the reverse of the order in which the corresponding receiver blocks were evaluated.
    ]

    on: selector do: action [
        The receiver is evaluated such that if during its evaluation an exception corresponding to
        selector is signaled then action will be evaluated. The result of evaluating the receiver is
        returned. 

        Before evaluating the receiver the current state of the exception environment is captured as the
        handler environment. Then a new exception handler is created with selector as its exception
        selector and action as its handler block. The new handler is pushed onto the exception
        environment.

        If evaluation of the receiver terminates normally then the exception environment is reset to the
        handler environment before returning to the sender of the #on:do: message.
        If signaling of an exception results in evaluation of action the evaluation will occur in the context
        of the handler environment. The argument to the action will be an object that conforms to the
        protocol <signaledException>.
    ]
]"


Object subclass: Exception [

]