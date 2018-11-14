package com.cangwang.magic.util

import android.content.res.AssetManager
import android.view.Surface

/**
 * openGL辅助类
 * Created by cangwang on 2018/10/12.
 */
object OpenGLJniLib{
    init {
        System.loadLibrary("magicjni")
    }

    /**
     * 初始化magicBaseView
     */
    external fun magicBaseInit(surface:Surface, width:Int, height:Int, manager: AssetManager):Int

    /**
     * 创建magicBaseView
     */
    external fun magicBaseCreate()

    /**
     * 变更触发magicBaseView
     */
    external fun magicBaseChange(width:Int,height:Int)

    /**
     * 绘制
     */
    external fun magicBaseDraw(matrix:FloatArray)

    external fun magicBaseRelease()

    /**
     * 调整大小
     */
    external fun magicAdjustSize(orientation:Int,isFront:Boolean,flipVertical:Boolean)
}