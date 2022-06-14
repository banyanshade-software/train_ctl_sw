#  LOW module

LOW module contains  "low level" tasklets, that can be dispatched on slave boards

- canton

- turnouts


Other slave tasklets are LED, and ina3221 handling, are not part of LOW

Canton power are controlled using both voltage level control and PWM, the aim being to reduce high frequency part
on the PWM output, while avoiding filtering the output. We thus keep reasonable "off" time to measure BEMF on each
PWM period
