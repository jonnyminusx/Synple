import { useEffect, useState, useCallback } from 'react'
import { getToggleState } from '../lib/juce/index.js'

export function useJuceToggle(id: string) {
  const [value, setValue] = useState(false)

  useEffect(() => {
    const state = getToggleState(id)
    setValue(state.getValue())

    const listenerId = state.valueChangedEvent.addListener(() => setValue(state.getValue()))
    return () => state.valueChangedEvent.removeListener(listenerId)
  }, [id])

  const setJuceValue = useCallback((v: boolean) => getToggleState(id).setValue(v), [id])

  return [value, setJuceValue] as const
}
