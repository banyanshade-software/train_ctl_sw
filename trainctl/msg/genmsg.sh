#!/bin/sh

G="../gen/msgdef_gen/genmsg"

$G h msg.def > trainmsgdef.h
$G s msg.def > trainmsgstr.c

